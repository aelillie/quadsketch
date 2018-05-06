#include "serialize.h"

#include <iostream>
#include <string>
#include <random>
#include <boost/program_options.hpp>
#include <chrono>
#include <stdexcept>
#include <vector>
#include <utility>
#include <functional>
#include <algorithm>
#include <limits>
#include <fstream>

using namespace std;
using namespace std::chrono;
using namespace ir;
namespace po = boost::program_options;

size_t n; //#points
int d; //Dimensions
const int c = 4; //Number of clusters
float min_value; //Minimum value in whole dataset
float max_value; //Maximum value in whole dataset
float aspect_ratio;
default_random_engine gen;

size_t num_queries;
const size_t SEED = 4057218;
const size_t K = 10;

int check_input(int argc, char **argv)
{
    //Description of arguments
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "print usage message")
        ("nodes,n", po::value<size_t>(), "number of nodes")
        ("dimensions,d", po::value<int>(), "number of dimensions")
        ("min,i", po::value<float>(), "min value")
        ("max,a", po::value<float>(), "max value")
        ("queries,q", po::value<size_t>(), "number of queries");
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
    if (!vm.count("nodes") || !vm.count("dimensions") || !vm.count("min") || !vm.count("max") || !vm.count("queries"))
    {
        cout << desc << endl;
        throw runtime_error("number of nodes and dimensions, and min and max values, and queries must be specified");
    }
    n = vm["nodes"].as<size_t>();
    d = vm["dimensions"].as<int>();
    min_value = vm["min"].as<float>();
    max_value = vm["max"].as<float>();
    num_queries = vm["queries"].as<size_t>();
    if (n <= 0)
    {
        cout << desc << endl;
        throw runtime_error("number of nodes must be positive");
    }
    if (d <= 0)
    {
        cout << desc << endl;
        throw runtime_error("dimensions must be positive");
    }
    if (max_value < min_value)
    {
        cout << desc << endl;
        throw runtime_error("max value must be greater than min value");
    }
    if (n < num_queries)
    {
        cout << desc << endl;
        throw runtime_error("number of queries must be less than number of nodes");
    }
    return 1;
}

void gen_rand_point(Point &p, 
                    normal_distribution<float> &dist1,
                    normal_distribution<float> &dist2) 
{
    for(int i = 0; i<d; ++i)
    {
        //Generate random coordinates in different ranges, 
        //as defined by the distributors
        if (i < d/2)
        {
            p[i] = dist1(gen);
        }
        else
        {
            p[i] = dist2(gen);        
        }
    }
}

void gen_datapoints(vector<Point> &dataset, 
                int start, int end, 
                normal_distribution<float> &dist1,
                normal_distribution<float> &dist2)
{
    //Generate a range of random points
    for(int i = start; i<end; ++i)
    {
        Point p(d);
        gen_rand_point(p, dist1, dist2);
        dataset[i] = p;
    }
}

void generate_queries(vector<Point> *dataset,
                      vector<Point> *queries) {
    mt19937_64 gen(SEED);
    queries->clear();
    for (size_t i = 0; i < num_queries; ++i) {
        uniform_int_distribution<> u(0, dataset->size() - 1);
        int ind = u(gen);
        queries->push_back((*dataset)[ind]);
        (*dataset)[ind] = dataset->back();
        dataset->pop_back();
    }
}

void generate_answers(const vector<Point> &dataset,
                      const vector<Point> &queries,
                      vector<vector<uint32_t>> *answers) {
    vector<pair<float, uint32_t>> scores(dataset.size());
    answers->resize(queries.size());
    float max_score = -1, min_score = numeric_limits<float>::infinity();
    size_t outer_counter = 0;
    for (const auto &query: queries) {
        size_t inner_counter = 0;
        for (const auto &data_point: dataset) {
            float score = (query - data_point).squaredNorm();
            scores[inner_counter] = make_pair(score, inner_counter);
            ++inner_counter;
            if (score < min_score)
            {
                min_score = score;
            }
            if (score > max_score)
            {
                max_score = score;
            }
        }
        nth_element(scores.begin(), scores.begin() + K - 1, scores.end());
        sort(scores.begin(), scores.begin() + K);
        (*answers)[outer_counter].resize(K);
        for (size_t i = 0; i < K; ++i) {
            (*answers)[outer_counter][i] = scores[i].second;
        }
        ++outer_counter;
        if (outer_counter % 100 == 0) {
            cout << outer_counter << "/" << queries.size() << endl;
        }
    }
    aspect_ratio = max_score/min_score;
}

void print_dataset(const vector<Point> &dataset)
{
    size_t size = dataset.size();
    for(size_t i = 0; i<size; ++i)
    {
        cout << i << ": " << flush;
        for(int j = 0; j<d; ++j)
        {
            cout << dataset[i][j] << ", " << flush;
        }
        cout << endl;
    }
    cout << endl;
}

//Verifies that the answers are properly generated
//by doing a basic nearest-neighbor search
void test_gen(vector<Point> &dataset,
              const vector<Point> &queries,
              vector<vector<uint32_t>> &answers)
{
    int q = queries.size();
    int size = n-q;
    for(auto x : queries)
    {
        dataset.push_back(x);
    }
    for (int i = 0; i < q; ++i)
    {
        float best_score = numeric_limits<float>::infinity();
        uint32_t who = -1;
        for (int j = 0; j < size; ++j)
        {
            float score = (dataset[j] - dataset[size + i]).squaredNorm();
            if (score < best_score)
            {
                best_score = score;
                who = j;
            }
        }
        if (who != answers[i][0])
            {
                cout << "Not right answer for query " << i << endl;
                cout << "Expected " << answers[i][0] << ", got " << who << endl;
            }
    }
}

int gen_datasets(vector<Point> &dataset, vector<Point> &queries, vector<vector<uint32_t>> &answers)
{
    try {
        cout << "generating queries" << endl;
        generate_queries(&dataset, &queries);
        cout << queries.size() << " points generated" << endl;

        cout << "writing queries" << endl;
        serialize("../../clusters/queries.dat", queries);
        cout << "done" << endl;
        //Note that the query points are removed from dataset at this point
        cout << "writing dataset" << endl;
        serialize("../../clusters/dataset.dat", dataset);
        cout << "done" << endl;

        cout << "generating answers" << endl;
        generate_answers(dataset, queries, &answers);
        cout << answers.size() << " points generated" << endl;
        
        cout << "writing answers" << endl;
        serialize("../../clusters/answers.dat", answers);
        cout << "done" << endl;
        
        cout << "Testing dataset..." << endl;
        test_gen(dataset, queries, answers);
        cout << "Done testing" << endl;
    }
    catch (runtime_error &e) {
        cerr << "runtime error: " << e.what() << endl;
        return 1;
    }
    catch (exception &e) {
        cerr << "exception: " << e.what() << endl;
        return 1;
    }
    catch (...) {
        cerr << "Unknown error" << endl;
        return 1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (!check_input(argc, argv))
    {
        return 0;
    }
    float range = max_value - min_value;
    float min_mean = min_value + range/8;
    float max_mean = max_value - range/8;

    cout << "n: " << n << endl;
    cout << "d: " << d << endl;
    cout << "min: " << min_value << endl;
    cout << "max: " << max_value << endl;
    cout << "range: " << range << endl;
    cout << "min_mean: " << min_mean << endl;
    cout << "max_mean: " << max_mean << endl;  

    normal_distribution<float> min_distr(min_mean, range/8);
    normal_distribution<float> max_distr(max_mean, range/8);

    vector<Point> dataset(n);

    gen_datapoints(dataset, 0      , n/4         , min_distr, min_distr);
    gen_datapoints(dataset, n/4    , n/2         , min_distr, max_distr);
    gen_datapoints(dataset, n/2    , (n*3)/4     , max_distr, min_distr);
    gen_datapoints(dataset, (n*3)/4, n           , max_distr, max_distr);

    vector<Point> queries;
    vector<vector<uint32_t>> answers;
    gen_datasets(dataset, queries, answers);

    string filename = "clusters_n-"+to_string(n)+"_d-"+to_string(d)+"_phi-"+to_string(aspect_ratio);
    ofstream output(filename);
    output << "n: " << n << endl;
    output << "d: " << d << endl;
    output << "aspect ratio: " << aspect_ratio << endl;    
    output << "min value: " << min_value << endl;
    output << "max value: " << max_value << endl;
    output << "range: " << range << endl;
    output << "min centroid: " << min_mean << endl;
    output << "max centroid: " << max_mean << endl;    
    output << "stddev: " << range/8 << endl;
    output.close();

    return 0;
}