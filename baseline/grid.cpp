#include "serialize.h"

#include <boost/program_options.hpp>

#include <chrono>
#include <fstream>
#include <map>
#include <iostream>
#include <random>
#include <vector>

using namespace std;
using namespace std::chrono;
using namespace ir;
namespace po = boost::program_options;

void grid(const vector<Point> &dataset, int landmarks) {
    
}

//Use this function to compare results and output
// void compare(string output_file,
//             string input_folder,
//             int landmarks,
//             const vector<Point> &dataset,
//             const vector<Point> &queries,
//             const vector<vector<uint32_t>> answers) {
//     //Write out statistics
//     ofstream output(output_file);
//     output << "method grid" << endl;
//     output << "dataset " << input_folder << endl;
//     output << "landmarks " << landmarks << endl;
//     // output << "size " << total << endl;
//     // output << "nn_accuracy " << (counter + 0.0) / (q + 0.0) << endl;
//     // output << "distortion " << (distortion + 0.0) / (q + 0.0) << endl;
//     output.close();
// }

int main(int argc, char **argv) {
    //Description of arguments
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "print usage message")
        ("input,i", po::value<string>(), "input dataset")
        ("output,o", po::value<string>(), "output file")
        ("landmarks,l", po::value<int>(), "landmarks")
        ("num_queries,q", po::value<int>(), "number of queries used for evaluation");
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
    if (!vm.count("input") || !vm.count("output") || !vm.count("landmarks") || !vm.count("num_queries"))
    {
        cout << desc << endl;
        throw runtime_error("input dataset, output file, number of landmarks, " +
            "and number of queries must be specified");
    }
    //No errors, save input parameters
    string input_folder = vm["input"].as<string>();
    string output_file = "../results/" + vm["output"].as<string>();
    int landmarks = vm["landmarks"].as<int>();
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
        int num_queries = vm["num_queries"].as<int>();
        if (num_queries > queries.size())
        {
            cout << desc << endl;
            throw runtime_error("too many queries");
        }
        queries.erase(queries.begin() + num_queries, queries.end()); //only use first q queries
    }
    int q = queries.size();
    for (auto x : queries) //auto type is automatically deduced from its initializer
    {
        dataset.push_back(x); //adds at the end
    }
    int n = dataset.size(); //number of points
    int d = dataset[0].size(); //dimensions: the number of coefficients, which is rows()*cols()
    cout << "Dataset size: " << n << ", point dimensions: " << d << endl;
    float cmin = 1e100; //123136 in decimal
    float cmax = -1e100;
    //find extreme values(min/max point values) for hyper cube
    for (int i = 0; i < d; ++i) //go through all point dimensions
    {
        for (int j = 0; j < n; ++j) //go through all points
        {
            cmin = min(cmin, dataset[j][i]);
            cmax = max(cmax, dataset[j][i]);
        }
    }
    // float delta = cmax - cmin; //delta range/"diameter" of hyper cube
    
    //grid(..);

    compare(output_file, input_folder, landmarks,
        dataset, queries, answers);
    return 0;
}
