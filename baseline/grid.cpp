#include "serialize.h"

#include <boost/program_options.hpp>

#include <chrono>
#include <fstream>
#include <map>
#include <set>
#include <iostream>
#include <random>
#include <vector>
#include <math.h>
#include <limits>

using namespace std;
using namespace std::chrono;
using namespace ir;
namespace po = boost::program_options;

vector<Point> new_dataset;

int grid(const vector<Point> &dataset, int dim, int landmarks, uint16_t *marks) {
	int n = dataset.size();
	float cmin = 1e100, cmax = -1e100;
	//Find smallest and biggest value in this dimension
	for (int i = 0; i < n; ++i)
	{
		cmin = min(cmin, dataset[i][dim]);
		cmax = max(cmax, dataset[i][dim]);
	}
	float delta = cmax - cmin; //range for this dimension
	float landmark_coords[landmarks]; //Placeholder for landmark coordinates
	float mark = cmin, space = delta / landmarks;
	//Create a vector of landmark coordinates
	for (int l = 0; l < landmarks; ++l) {
		landmark_coords[l] = mark;
		mark += space;
	}
	//Round off all points in this dimension to nearest landmark
    set<uint16_t> count;
	for (int i = 0; i < n; ++i) {
		float dist_min = numeric_limits<float>::infinity();;
		int landmark = -1;
		for (int l = 0; l<landmarks; ++l) {
            //NOTE: The following may not be the best float comparisons
			float dist = fabs(dataset[i][dim] - landmark_coords[l]);
			if (dist < dist_min) {
				dist_min = dist;
				landmark = l;
			}
		}
        count.insert(marks[landmark]);
		new_dataset[i][dim] = marks[landmark]; //TODO: Pointer or just int?
	}
    return count.size();
}

//Use this function to compare results and output
 void compare(string output_file,
             string input_folder,
             int landmarks,
             const vector<Point> &dataset,
             const vector<Point> &queries,
             const vector<vector<uint32_t>> answers) {
	 int counter = 0;
	 double distortion = 0.0;
	 int q = queries.size(), n = dataset.size();
	 for (int i = 0; i < q; ++i) {
         /* Find the index of the compressed point
         that is closest to the compressed query point
         in euclidean distance */
		 float best_score = 1e100;
		 uint32_t who = -1;
		 for (int j = 0; j < n - q; ++j) {
             /* Find distance between the int vectors,
             that is the landmarks for data point j,
             and query point i.
             Note that the compressed query points are 
             stored in the same vector as the compressed
             data points */
			 float score = (new_dataset[j] - new_dataset[n - q + i]).squaredNorm();
			 if (score < best_score) {
				 best_score = score;
				 who = j;
			 }
		 }
        /* Check if the query point is actually already in 
        the original dataset, within some delta precision
        answers[i][0] is the true nearest neighbor, while
        answers[i][1...K] are the K nearest neighbors(sorted) */
		 float dd = (dataset[answers[i][0]] - queries[i]).norm();
		 if (dd < 1e-3) {
			 distortion += 1.0; //No distortion, as it is the same
			 ++counter; //Perfect accuracy
			 cout << "=" << flush;
		 }
         /*otherwise, we check if the compressed nearest neighbor
         is the true nearest neighbor */
		 else { 
			 distortion += (dataset[who] - queries[i]).norm() / (dataset[answers[i][0]] - queries[i]).norm();
			 if (who == answers[i][0]) //The indices match, this it is a hit
			 {
				 ++counter;
				 cout << "+" << flush;
			 }
			 else //The compressed NN was incorrect
			 {
				 cout << "-" << flush;
			 }
		 }
	 }
	 cout << endl;
	 double dist = (distortion + 0.0) / (q + 0.0);
	 double accuracy = (counter + 0.0) / (q + 0.0);
	 cout << "accuracy " << accuracy << endl;
	 cout << "distortion " << dist << endl;
     //Write out statistics to file
     ofstream output(output_file);
     output << "method grid" << endl;
     output << "dataset " << input_folder << endl;
     output << "landmarks " << landmarks << endl;
     output << "nn_accuracy " << accuracy << endl;
     output << "distortion " << dist << endl;
     output.close();
 }

int main(int argc, char **argv) {
    //Description of arguments
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "print usage message")
        ("input,i", po::value<string>(), "input dataset")
        ("output,o", po::value<string>(), "output file")
        ("landmarks,l", po::value<uint16_t>(), "landmarks")
        ("num_queries,q", po::value<size_t>(), "number of queries used for evaluation");
    po::variables_map vm; //variables map derived from std::map<std::string, variable_value>
    //cause vm to contain all the options found on the command line
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    //count checks whether a certain command-line option has been used and is stored in the container
    if (vm.count("help"))
    {
        cout << desc << endl;
        return 0;
    }
    if (!vm.count("input") || !vm.count("output") || !vm.count("landmarks"))
    {
        cout << desc << endl;
        throw runtime_error("input dataset, output file, number of landmarks, "
            "and number of queries must be specified");
    }
    //No errors, save input parameters
    string input_folder = vm["input"].as<string>();
    string output_file = "../results/" + vm["output"].as<string>();
    uint16_t landmarks = vm["landmarks"].as<uint16_t>();
    if (landmarks <= 0)
    {
        cout << desc << endl;
        throw runtime_error("number of landmarks must be positive");
    }

    //Populate data vectors
    vector<Point> dataset;
    vector<Point> queries;
    vector<vector<uint32_t>> answers;
    deserialize(input_folder + "/dataset.dat", &dataset); //Input data
    deserialize(input_folder + "/queries.dat", &queries); //Queries on data
    deserialize(input_folder + "/answers.dat", &answers); //Comparison data
    if (vm.count("num_queries"))
    {
        size_t num_queries = vm["num_queries"].as<size_t>();
        if (num_queries > queries.size())
        {
            cout << desc << endl;
            throw runtime_error("too many queries");
        }
        queries.erase(queries.begin() + num_queries, queries.end()); //only use first q queries
    }
    for (auto x : queries) //auto type is automatically deduced from its initializer
    {
        dataset.push_back(x); //adds at the end
    }
    int n = dataset.size(); //number of points
    int d = dataset[0].size(); //dimensions: the number of coefficients, which is rows()*cols()
    cout << "Dataset size: " << n << ", point dimensions: " << d << endl;
	//create vector with same lengths as dataset
	new_dataset.resize(n);
	for (int i = 0; i < n; ++i)
	{
		new_dataset[i].resize(d); //make space for a Point
	}
    uint16_t marks[landmarks];
    for(uint16_t mark = 0; mark<landmarks; ++mark) {
        marks[mark] = mark;
    }
	int total = 0;
	for (int dim = 0; dim < d; ++dim) {
		total += grid(dataset, dim, landmarks, marks);
	}
    cout << "total: " << total << endl;
    cout << "average landmarks pr. dimension: " << (total*1.0)/(d*1.0) << endl;
    compare(output_file, input_folder, landmarks, dataset, queries, answers);
    return 0;
}
