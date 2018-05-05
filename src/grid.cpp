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
#include <thread>

using namespace std;
using namespace std::chrono;
using namespace ir;
namespace po = boost::program_options;

vector<Point> dataset;
vector<Point> new_dataset;
vector<pair<float, float>> min_max;
int threads;
uint16_t landmarks;

void grid(int dim, vector<uint16_t> marks)
{
    int n = dataset.size();
    float cmin = 1e100, cmax = -1e100;
    //Find smallest and biggest value in this dimension
    for (int i = 0; i < n; ++i)
    {
        cmin = min(cmin, dataset[i][dim]);
        cmax = max(cmax, dataset[i][dim]);
    }
    float delta = cmax - cmin; //range for this dimension
    min_max[dim].first = cmin;
    min_max[dim].second = cmax;
    float landmark_coords[landmarks]; //Placeholder for landmark coordinates
    float mark = cmin, space = delta / landmarks;
    //Create a vector of landmark coordinates
    for (int l = 0; l < landmarks; ++l)
    {
        landmark_coords[l] = mark;
        mark += space;
    }
    //Round off all points in this dimension to nearest landmark
    for (int i = 0; i < n; ++i)
    {
        float dist_min = numeric_limits<float>::infinity();
        int landmark = -1;
        for (int l = 0; l < landmarks; ++l)
        {
            //NOTE: The following may not be the best float comparisons
            float dist = fabs(dataset[i][dim] - landmark_coords[l]);
            if (dist < dist_min)
            {
                dist_min = dist;
                landmark = l;
            }
        }
        new_dataset[i][dim] = marks[landmark]; //TODO: Pointer or just int?
    }
}

void run_grid(int start, int end, vector<uint16_t> marks) 
{
    for (int dim = start; dim < end; ++dim)
    {
        grid(dim, marks);
    }    
}

void decompress()
{
    int n = dataset.size(), dim = new_dataset[0].size();
    for (int j = 0; j < n; ++j)
    {
        for (int d = 0; d < dim; ++d)
        {
            /* Approximate coordinate from landmark,
            by computing (min+((max-min)/landmarks)*landmark) */
            new_dataset[j][d] = min_max[d].first +
                                ((min_max[d].second - min_max[d].first) / landmarks) * new_dataset[j][d];
        }
    }
}

void run_compare(int start, int end, 
                const vector<Point> &queries, 
                const vector<vector<uint32_t>> answers,
                vector<int> &counter, 
                vector<double> &distortion, 
                int t)
{
    cout << "Thread " << t << " start" << endl;
    int q = queries.size(), n = dataset.size();
    for (int i = start; i < end; ++i)
    {
        /* Find nearest neighbor to query point */
        float best_score = 1e100;
        uint32_t who = -1;
        for (int j = 0; j < n - q; ++j)
        {
            float score = (new_dataset[j] - new_dataset[n - q + i]).squaredNorm();
            if (score < best_score)
            {
                best_score = score;
                who = j;
            }
        }

        /* Check if the query point is actually already in 
        the original dataset, within some delta precision
        answers[i][0] is the true nearest neighbor, while
        answers[i][1...K] are the K nearest neighbors(sorted) */
        float dd = (dataset[answers[i][0]] - queries[i]).norm();
        if (dd < 1e-3)
        {
            distortion[t] += 1.0; //No distortion, as it is the same
            counter[t]++;         //Perfect accuracy
            //cout << "=" << flush;
        }
        /*otherwise, we check if the compressed nearest neighbor
         is the true nearest neighbor */
        else
        {
            distortion[t] += (dataset[who] - queries[i]).norm() / (dataset[answers[i][0]] - queries[i]).norm();
            if (who == answers[i][0]) //The indices match, this it is a hit
            {
                counter[t]++;
                //cout << "+" << flush;
            }
            else //The compressed NN was incorrect
            {
                //cout << "-" << flush;
            }
        }
    }
    cout << "Thread " << t << " end. " << endl;
}

//Use this function to compare results and output
void compare(string output_file,
             string input_folder,
             const vector<Point> &queries,
             const vector<vector<uint32_t>> answers,
             double size)
{
    vector<int> counters(threads, 0);
    vector<double> distortions(threads, 0.0);
    int q = queries.size();
    cout << "Decompressing..." << endl;
    decompress(); //decompress all points
    cout << "Done decompressing..." << endl;
    cout << "Starting nearest neighbor search..." << endl;
    thread compare_threads[threads];
    int start = 0, m = q/threads, end = m;
    for(int i = 0; i < threads; ++i)
    {
        compare_threads[i] = 
            thread(run_compare, start, end, queries, answers, ref(counters), ref(distortions), i);
        start = end;
        if (i == threads-2) //Account for uneven threads
        {
            end = q;
        }
        else
        {
            end += m; 
        }
    }
    for (int i=0; i<threads; i++){
        compare_threads[i].join();
    }
    cout << "Done" << endl;
    int counter = 0;
    double distortion = 0.0;
    for(auto c : counters) {
        counter += c;
    }
    for(auto d : distortions) {
        distortion += d;
    }
    cout << endl;
    double dist = (distortion + 0.0) / (q + 0.0);
    double accuracy = (counter + 0.0) / (q + 0.0);
    cout << "accuracy " << accuracy << endl;
    cout << "distortion " << dist << endl;
    cout << "bits/coord: " << size << endl;
    //Write out statistics to file
    ofstream output(output_file, ios_base::app);
    output << "grid," << input_folder << "," << landmarks << "," << accuracy << "," << dist << "," << size << endl;
    output.close();
}

int main(int argc, char **argv)
{
    //Description of arguments
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "print usage message")
        ("input,i", po::value<string>(), "input dataset")
        ("output,o", po::value<string>(), "output file")
        ("landmarks,l", po::value<uint16_t>(), "landmarks")
        ("num_queries,q", po::value<size_t>(), "number of queries used for evaluation")
        ("threads,t", po::value<int>(), "number of threads");
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
    string output_file = "grid_results/" + vm["output"].as<string>();
    landmarks = vm["landmarks"].as<uint16_t>();
    if (landmarks <= 0)
    {
        cout << desc << endl;
        throw runtime_error("number of landmarks must be positive");
    }

    //Populate data vectors
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
    if (vm.count("threads"))
    {
        threads = vm["threads"].as<int>();
    }
    else 
    {
        threads = 4;
    }
    //Append query points to the end of dataset
    for (auto x : queries) 
    {
        dataset.push_back(x);
    }
    int q = queries.size();
    int n = dataset.size();    //number of points
    int d = dataset[0].size(); //dimensions: the number of coefficients, which is rows()*cols()
    //create vector with same lengths as dataset
    new_dataset.resize(n);
    for (int i = 0; i < n; ++i)
    {
        new_dataset[i].resize(d); //make space for a Point
    }
    min_max.resize(d);
    vector<uint16_t> marks(landmarks);
    for (uint16_t mark = 0; mark < landmarks; ++mark)
    {
        marks[mark] = mark;
    }
    cout << "Dataset size: " << (n-q) << ", point dimensions: " << d << endl;
    cout << "Queries: " << q << endl;
    cout << "Compressing..." << endl;
    thread grid_threads[threads];
    int start = 0, m = d/threads, end = m;
    for(int i = 0; i < threads; ++i)
    {
        grid_threads[i] = thread(run_grid, start, end, marks);
        start = end;
        if (i == threads-2) //Account for uneven threads
        {
            end = d;
        }
        else
        {
            end += m; 
        }
    }
    for (int i=0; i<threads; i++){
        grid_threads[i].join();
    }
    double size = log2(landmarks*1.0);
    cout << "Done compressing..." << endl;
    cout << "bits/coord: " << size << endl;
    compare(output_file, input_folder, queries, answers, size);
    return 0;
}
