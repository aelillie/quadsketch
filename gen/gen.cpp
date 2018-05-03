#include <iostream>
#include <string>
#include <random>
#include <boost/program_options.hpp>
#include "serialize.h"
#include <chrono>
#include <stdexcept>
#include <vector>
#include <utility>
#include <functional>
#include <algorithm>

using namespace std;
using namespace std::chrono;
using namespace ir;
namespace po = boost::program_options;

int n;
int d;
int c = 4;
float min_value;
float max_value;
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
        ("nodes,n", po::value<int>(), "number of nodes")
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
    n = vm["nodes"].as<int>();
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

void gen_dataset(vector<Point> &dataset, 
                int start, int end, 
                normal_distribution<float> &dist1,
                normal_distribution<float> &dist2)
{
    
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
    greater<pair<float, uint32_t>> comparator;
    answers->resize(queries.size());
    size_t outer_counter = 0;
    for (const auto &query: queries) {
        size_t inner_counter = 0;
        for (const auto &data_point: dataset) {
            float score = query.dot(data_point);
            scores[inner_counter] = make_pair(score, inner_counter);
            ++inner_counter;
        }
        nth_element(scores.begin(), scores.begin() + K - 1, scores.end(), comparator);
        sort(scores.begin(), scores.begin() + K, comparator);
        (*answers)[outer_counter].resize(K);
        for (size_t i = 0; i < K; ++i) {
            (*answers)[outer_counter][i] = scores[i].second;
        }
        ++outer_counter;
        if (outer_counter % 100 == 0) {
            cout << outer_counter << "/" << queries.size() << endl;
        }
    }
}

void print_dataset(const vector<Point> &dataset)
{
    for(int i = 0; i<n; ++i)
    {
        for(int j = 0; j<d; ++j)
        {
            cout << dataset[i][j] << ", " << flush;
        }
        cout << endl;
    }
    cout << endl;
}

int step1(vector<Point> dataset) 
{
    try {
        cout << "writing dataset" << endl;
        serialize("dataset.dat", dataset);
        cout << "done" << endl;
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

int step2()
{
    try {
        vector<Point> dataset;
        cout << "reading dataset" << endl;
        deserialize("dataset.dat", &dataset);
        cout << "done" << endl;
        cout << dataset.size() << " points read" << endl;

        vector<Point> queries;

        cout << "generating queries" << endl;
        generate_queries(&dataset, &queries);
        cout << "done" << endl;
        cout << queries.size() << " points generated" << endl;

        cout << "writing queries" << endl;
        serialize("queries.dat", queries);
        cout << "done" << endl;

        vector<vector<uint32_t>> answers;

        cout << "generating answers" << endl;
        generate_answers(dataset, queries, &answers);
        cout << "done" << endl;
        cout << answers.size() << " points generated" << endl;
        
        cout << "writing answers" << endl;
        serialize("answers.dat", answers);
        cout << "done" << endl;
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
    cout << "n: " << n << endl;
    cout << "d: " << d << endl;
    cout << "min: " << min_value << endl;
    cout << "max: " << max_value << endl;
    float range = max_value - min_value;
    float min_mean = range/8;
    float max_mean = range*7/8;
    normal_distribution<float> min_distr(min_mean, range/8);
    normal_distribution<float> max_distr(max_mean, range/8);

    vector<Point> dataset(n);

    gen_dataset(dataset, 0      , n/4         , min_distr, min_distr);
    gen_dataset(dataset, n/4    , n/2         , min_distr, max_distr);
    gen_dataset(dataset, n/2    , (n*3)/4     , max_distr, min_distr);
    gen_dataset(dataset, (n*3)/4, n           , max_distr, max_distr);

    //print_dataset(dataset);

    step1(dataset);
    step2();

    return 0;
}