#include "serialize.h"

#include <boost/program_options.hpp>

#include <chrono>
#include <fstream>
#include <map>
#include <iostream>
#include <random>
#include <vector>
#include <thread>

using namespace std;
using namespace std::chrono;
using namespace ir;
namespace po = boost::program_options;

vector<Point> dataset;
vector<Point> new_dataset;
int threads = 4;

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
    for (auto x : who)
    {
        vector<uint8_t> code(dd, 0);
        for (int j = 0; j < d; ++j)
        {
            if (dataset[x][b1 + j] < mm[j])
            { //Why empty if-body!?!?
            }
            else
            {
                code[j / 8] |= 1 << (j % 8); //bitwise magic
            }
        }
        parts[code].push_back(x);
    }
    for (auto x : parts)
    {
        vector<pair<float, float>> newbb(bb.size());
        Point new_p(cur);
        for (int j = 0; j < d; ++j)
        {
            newbb[j] = bb[j];
            if ((x.first[j / 8] >> (j % 8)) & 1)
            {
                newbb[j].first = mm[j];
                if (long_edge_length >= lambda && parts.size() == 1)
                {
                    if (rand()%2)
                    {
                        new_p[j] += 1.0 / (2.0 * (1 << depth));
                    }
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
        { //yet another empty if?!?
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

void run_nn(int start, int end, 
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

int main(int argc, char **argv)
{
    //Description of arguments
    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "print usage message")("input,i", po::value<string>(), "input dataset")("output,o", po::value<string>(), "output file")("depth,d", po::value<int>(), "depth of a tree")("num_blocks,n", po::value<int>(), "number of blocks")("lambda,l", po::value<int>(), "compression parameter")("num_queries,q", po::value<size_t>(), "number of queries used for evaluation")("threads,t", po::value<int>(), "number of threads");
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
    if (vm.count("threads"))
    {
        threads = vm["threads"].as<int>();
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
    if (threads < 1)
    {
        cout << desc << endl;
        throw runtime_error("number of threads must be positive");
    }
    random_device rd;     //integer random number generator
    mt19937_64 gen(rd()); //random_device implements the mt19937_64 64-bit number generator
    vector<Point> queries;
    vector<vector<uint32_t>> answers;
    //Populate data vectors
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
    //Find aspect ratio
    // float lowest = 1e100, highest = -1e100;
    // for(int i = 0; i<n; ++i) {
    //     for (int j = i+1; j < n; ++j)
    //     {
    //         float score = (dataset[i] - dataset[j]).norm();
    //         if (score < lowest)
    //         {
    //             lowest = score;
    //         }
    //         if (score > highest)
    //         {
    //             highest = score;
    //         }
    //     }
    // }
    // float a_r = highest/lowest;
    float a_r = 0.0;
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
            ++t;
        }
        total += n * t;
        t = 0;
        while (1ll << t < max_reduced_edge)
        {
            ++t;
        }
        total += num_reduced_edges * t;
    }
    vector<int> counters(threads, 0);
    vector<double> distortions(threads, 0.0);
    cout << "Starting nearest neighbor search." << endl;
    cout << "Running on " << threads << " threads..." << endl;
    thread compare_threads[threads];
    int t_start = 0, m = q/threads, t_end = m;
    for(int i = 0; i < threads; ++i)
    {
        compare_threads[i] = 
            thread(run_nn, t_start, t_end, queries, answers, ref(counters), ref(distortions), i);
        t_start = t_end;
        if (i == threads-2) //countermeasure for uneven threads
        {
            t_end = q;
        }
        else
        {
            t_end += m; 
        }
    }
    for (int i=0; i<threads; i++){
        compare_threads[i].join();
    }
    cout << "Done" << endl;
    int counter = 0;
    double distortion = 0.0;
    for(int i = 0; i<threads; ++i) {
        counter += counters[i];
        distortion += distortions[i];
    }
    double accuracy = (counter + 0.0) / (q + 0.0);
    distortion /= q*1.0;
    double bc = (total*1.0/((n*1.0)*(d*1.0)));
    cout << "accuracy: " << accuracy << endl;
    cout << "distortion: " << distortion<< endl;
    cout << "aspect ratio: " << a_r << endl;
    ofstream output(output_file, ios_base::app);
    output << "qsr," << total << "," << lambda << "," << depth << "," << input_folder << "," 
        << bc << "," << accuracy << "," << distortion << "," << d << "," << a_r << "," << num_blocks << endl;
    output.close();
    //size,lambda,depth,dataset,B,accuracy,distortion,dim,aspectRatio,blocks
    return 0;
}
