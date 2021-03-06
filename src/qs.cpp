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

vector<Point> new_dataset;

void solve(const vector<Point> &dataset, //initally the complete input dataset
           int b1, //block start
           int b2, //block end
           const vector<pair<float, float>> &bb, //a random point range in the hyper cube
           const vector<int> &who, //the 'all' vector
           int depth, //initially 0
           int max_depth, //does not change
           uint64_t *num_leaves, //initially 0
           uint64_t *dfs_size, //initially 0
           uint64_t *reduced_dfs_size, //initially 0
           uint64_t *num_reduced_edges, //initially 0
           int *max_reduced_edge, //initially 0
           const Point &cur, //initially a Point with all 0's
           int long_edge_length, //initially 0
           int lambda)
{
    int n = dataset.size();
    int d = b2 - b1; //block length
    int dd = d / 8; //divide block
    if (d % 8) //check if there is excess space
        ++dd; //if so, make space for 1 more byte
    if (long_edge_length - lambda >= *max_reduced_edge)
    {
        *max_reduced_edge = long_edge_length - lambda;
    }
    if (depth >= max_depth) //We have reached the max depth
    {
        //Make leaf
        ++(*num_leaves);
        for (auto x : who)
        {
            for (int i = 0; i < d; ++i)
            {
                new_dataset[x][b1 + i] = cur[i];
            }
        }
        return; //Terminates recursion for 'solve'
    }
    vector<float> mm; //Random point
    for (auto x : bb)
    {
        //Populate mm with random coordinates
        mm.push_back((x.first + x.second) / 2.0);
    }
    map<vector<uint8_t>, vector<int>> parts;
    //Make splitting of the points, based
    //on if they are below or above random point
    for (auto x : who)
    {
        vector<uint8_t> code(dd, 0);
        for (int j = 0; j < d; ++j)
        {
            if (dataset[x][b1 + j] < mm[j])
            { 
            }
            else
            {
                //adds a bit from right to left
                code[j / 8] |= 1 << (j % 8);
            }
        }
        parts[code].push_back(x);
    }
    for (auto x : parts)
    {
        vector<pair<float, float>> newbb(bb.size());
        Point new_p(cur);
        //Create a new random point based on the prior,
        //and assign new leaf values if necessary
        for (int j = 0; j < d; ++j)
        {
            newbb[j] = bb[j];
            if ((x.first[j / 8] >> (j % 8)) & 1)
            {
                newbb[j].first = mm[j];
                if (long_edge_length >= lambda && parts.size() == 1)
                {
                    continue;
                }
                new_p[j] += 1.0 / (2.0 * (1 << depth));
            }
            else
            {
                newbb[j].second = mm[j];
            }
        }
        (*dfs_size) += 2 + d;
        (*reduced_dfs_size) += 2;
        if (long_edge_length >= lambda && parts.size() == 1)
        { 
        }
        else
        {
            (*reduced_dfs_size) += 1 + d;
        }
        if (long_edge_length == lambda && parts.size() == 1)
        {
            ++(*num_reduced_edges);
            (*reduced_dfs_size) += 1;
        }
        solve(dataset,
              b1,
              b2,
              newbb,
              x.second,
              depth + 1,
              max_depth,
              num_leaves,
              dfs_size,
              reduced_dfs_size,
              num_reduced_edges,
              max_reduced_edge,
              new_p,
              (parts.size() > 1) ? 1 : long_edge_length + 1,
              lambda);
    }
}

int main(int argc, char **argv)
{
    //Description of arguments
    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "print usage message")("input,i", po::value<string>(), "input dataset")("output,o", po::value<string>(), "output file")("depth,d", po::value<int>(), "depth of a tree")("num_blocks,n", po::value<int>(), "number of blocks")("lambda,l", po::value<int>(), "compression parameter")("num_queries,q", po::value<int>(), "number of queries used for evaluation");
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
    if (!vm.count("input") || !vm.count("output") || !vm.count("depth") || !vm.count("num_blocks") || !vm.count("lambda"))
    {
        cout << desc << endl;
        throw runtime_error("input dataset, output file, number of blocks, the compression parameter and depth of a tree must be specified");
    }
    //No errors, save input parameters
    string input_folder = vm["input"].as<string>();
    string output_file = "../dataAnalysis/" + vm["output"].as<string>();
    int num_blocks = vm["num_blocks"].as<int>();
    int depth = vm["depth"].as<int>();
    int lambda = vm["lambda"].as<int>();
    if (num_blocks <= 0)
    {
        cout << desc << endl;
        throw runtime_error("number of blocks must be positive");
    }
    if (depth <= 0)
    {
        cout << desc << endl;
        throw runtime_error("depth must be positive");
    }
    if (lambda <= 0)
    {
        cout << desc << endl;
        throw runtime_error("lambda must be positive");
    }
    if (lambda > depth)
    {
        cout << desc << endl;
        throw runtime_error("lambda must be at most depth");
    }
    random_device rd;     //integer random number generator
    mt19937_64 gen(rd()); //random_device implements the mt19937_64 64-bit number generator
    vector<Point> dataset;
    vector<Point> queries;
    vector<vector<uint32_t>> answers;
    //Populate data vectors
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
    float delta = cmax - cmin; //delta range/"diameter" of hyper cube
    cmin -= delta;
    uniform_real_distribution<float> u(0.0, delta); //returns random floating-point between 0.0 and delta
    //initialize int vector of same length as data set with index identity values
    vector<int> all(n);
    for (int i = 0; i < n; ++i)
    {
        all[i] = i;
    }
    int start = 0;
    uint64_t total = 0;
    //create vector with same lengths as dataset
    new_dataset.resize(n);
    for (int i = 0; i < n; ++i)
    {
        new_dataset[i].resize(d); //make space for a Point
    }
    for (int block_id = 0; block_id < num_blocks; ++block_id)
    {
        int block_len = d / num_blocks; //blocks the dimensions
        if (block_id < d % num_blocks) 
        {
            ++block_len;
        }
        cout << "block start: " << start << ", block end: " << start + block_len << endl;
        vector<pair<float, float>> bb; //A random point range within delta
        for (int i = 0; i < d; ++i)
        {
            float s = u(gen); //random floating-point between 0.0 and delta
            bb.push_back(make_pair(cmin + s, cmax + s)); //random coordinate in dimensions i. 0<i<d
        }
        uint64_t num_leaves = 0;
        uint64_t dfs_size = 0;
        uint64_t reduced_dfs_size = 0;
        uint64_t num_reduced_edges = 0;
        int max_reduced_edge = 0;
        solve(dataset, start, start + block_len, bb, all, 0, depth, &num_leaves, &dfs_size, &reduced_dfs_size, &num_reduced_edges, &max_reduced_edge, Point::Zero(block_len), 0, lambda);
        cout << "Num leaves: " <<num_leaves << ", dfs size: " << reduced_dfs_size << ", reduced edges: " << num_reduced_edges << ", max reduced edge: " << max_reduced_edge << endl;
        start += block_len;
        total += reduced_dfs_size;
        int t = 0;
        while (1ll << t < num_leaves)
        {
            ++t; //#branches from root
        }
        total += n * t;
        t = 0;
        while (1ll << t < max_reduced_edge)
        {
            ++t;
        }
        total += num_reduced_edges * t;
    }
    cout << total << endl;
    int counter = 0;
    double distortion = 0.0;
    for (int i = 0; i < q; ++i)
    {
        float best_score = 1e100;
        int who = -1;
        for (int j = 0; j < n - q; ++j)
        {
            float score = (new_dataset[j] - new_dataset[n - q + i]).squaredNorm();
            if (score < best_score)
            {
                best_score = score;
                who = j;
            }
        }
        float dd = (dataset[answers[i][0]] - queries[i]).norm();
        if (dd < 1e-3)
        {
            distortion += 1.0;
            ++counter;
            cout << "+" << flush;
        }
        else
        {
            distortion += (dataset[who] - queries[i]).norm() / (dataset[answers[i][0]] - queries[i]).norm();
            if (who == answers[i][0])
            {
                ++counter;
                cout << "+" << flush;
            }
            else
            {
                cout << "-" << flush;
            }
        }
    }
    cout << endl;
    cout << "Counts per query: " << (counter + 0.0) / (q + 0.0) << endl;
    cout << "distortion " << (distortion + 0.0) / (q + 0.0) << endl;
    ofstream output(output_file);
    //output order relevant for automatic read of generated files. 
    output << "method qs" << endl;
    output << "dataset " << input_folder << endl;
    output << "num_blocks " << num_blocks << endl;
    output << "depth " << depth << endl;
    output << "lambda " << lambda << endl;
    output << "size " << total << endl;
    output << "delta " << delta << endl;
    output << "dim " << d << endl;
    output << "nn_accuracy " << (counter + 0.0) / (q + 0.0) << endl;
    output << "distortion " << (distortion + 0.0) / (q + 0.0) << endl;
    output.close();
    return 0;
}
