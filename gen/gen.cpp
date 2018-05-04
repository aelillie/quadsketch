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
#include <limits>

using namespace std;
using namespace std::chrono;
using namespace ir;
namespace po = boost::program_options;

size_t n;
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

// void generate_answers(const vector<Point> &dataset,
//                       const vector<Point> &queries,
//                       vector<vector<uint32_t>> *answers) {
//     vector<pair<float, uint32_t>> scores(dataset.size());
//     greater<pair<float, uint32_t>> comparator;
//     answers->resize(queries.size());
//     size_t outer_counter = 0;
//     for (const auto &query: queries) {
//         size_t inner_counter = 0;
//         for (const auto &data_point: dataset) {
//             float score = query.dot(data_point);
//             scores[inner_counter] = make_pair(score, inner_counter);
//             ++inner_counter;
//         }
//         nth_element(scores.begin(), scores.begin() + K - 1, scores.end(), comparator);
//         sort(scores.begin(), scores.begin() + K, comparator);
//         (*answers)[outer_counter].resize(K);
//         cout << outer_counter << ": " << flush;
//         for (size_t i = 0; i < K; ++i) {
//             (*answers)[outer_counter][i] = scores[i].second;
//             cout << i << ":" << scores[i].second << ", " << flush;
//         }
//         cout << endl;
//         ++outer_counter;
//         if (outer_counter % 100 == 0) {
//             cout << outer_counter << "/" << queries.size() << endl;
//         }
//     }
// }
void generate_answers(const vector<Point> &dataset,
                      const vector<Point> &queries,
                      vector<vector<uint32_t>> *answers) {
    vector<pair<float, uint32_t>> scores(dataset.size());
    answers->resize(queries.size());
    size_t outer_counter = 0;
    for (const auto &query: queries) {
        size_t inner_counter = 0;
        for (const auto &data_point: dataset) {
            float score = (query - data_point).squaredNorm();
            scores[inner_counter] = make_pair(score, inner_counter);
            ++inner_counter;
        }
        nth_element(scores.begin(), scores.begin() + K - 1, scores.end());
        sort(scores.begin(), scores.begin() + K);
        (*answers)[outer_counter].resize(K);
        // cout << outer_counter << ": " << flush;
        for (size_t i = 0; i < K; ++i) {
            (*answers)[outer_counter][i] = scores[i].second;
            // cout << i << ":" << scores[i].second << ", " << flush;
            //cout << dataset[scores[i].second].dot(query) / query.norm() / dataset[scores[i].second].norm() << " ";
        }
        // cout << endl;
        ++outer_counter;
        if (outer_counter % 100 == 0) {
            cout << outer_counter << "/" << queries.size() << endl;
        }
    }
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

void test_gen(vector<Point> &dataset,
              const vector<Point> &queries,
              vector<vector<uint32_t>> &answers)
{
    int q = queries.size();
    for(auto x : queries)
    {
        dataset.push_back(x);
    }
    for (int i = 0; i < q; ++i)
    {
        float best_score = numeric_limits<float>::infinity();
        int who = -1;
        // for(int z = 0; z<d; ++z)
        // {
        //     cout << dataset[n - q + i][z] << " " << flush;
        // }
        // cout << endl;
        for (int j = 0; j < n - q; ++j)
        {
            float score = (dataset[j] - dataset[n - q + i]).squaredNorm();
            // cout << "score " << j << ": " << score << ", " << flush;
            if (score < best_score)
            {
                best_score = score;
                who = j;
            }
        }
        // cout << endl;
        // cout << i << ", who: " << who << endl;
        float dd = (dataset[answers[i][0]] - queries[i]).norm();
        if (dd < 1e-3)
        {
            // distortion += 1.0;
            // ++counter;
            cout << "+" << flush;
        }
        else
        {
            // distortion += (dataset[who] - queries[i]).norm() / (dataset[answers[i][0]] - queries[i]).norm();
            if (who == answers[i][0])
            {
                // ++counter;
                cout << "+" << flush;
            }
            else
            {
                cout << "-" << flush;
            }
        }
    }
}

// int step1(vector<Point> &dataset) 
// {
//     try {
//         cout << "writing dataset" << endl;
//         serialize("dataset.dat", dataset);
//         cout << "done" << endl;
//     }
//     catch (runtime_error &e) {
//         cerr << "runtime error: " << e.what() << endl;
//         return 1;
//     }
//     catch (exception &e) {
//         cerr << "exception: " << e.what() << endl;
//         return 1;
//     }
//     catch (...) {
//         cerr << "Unknown error" << endl;
//         return 1;
//     }
//     return 0;
// }

int gen_datasets(vector<Point> &dataset, vector<Point> &queries, vector<vector<uint32_t>> &answers)
{
    try {
        cout << "generating queries" << endl;
        generate_queries(&dataset, &queries);
        cout << "done" << endl;
        cout << queries.size() << " points generated" << endl;

        cout << "writing queries" << endl;
        serialize("queries.dat", queries);
        cout << "done" << endl;

        cout << "writing dataset" << endl;
        serialize("dataset.dat", dataset);
        cout << "done" << endl;

        cout << "generating answers" << endl;
        generate_answers(dataset, queries, &answers);
        cout << "done" << endl;
        cout << answers.size() << " points generated" << endl;
        
        cout << "writing answers" << endl;
        serialize("answers.dat", answers);
        cout << "done" << endl;

        // cout << "Dataset:" << endl;
        // print_dataset(dataset);
        
        test_gen(dataset, queries, answers);
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
    cout << "range: " << range << endl;
    float min_mean = min_value + range/8;
    cout << "min_mean: " << min_mean << endl;
    float max_mean = max_value - range/8;
    cout << "max_mean: " << max_mean << endl;
    normal_distribution<float> min_distr(min_mean, range/8);
    normal_distribution<float> max_distr(max_mean, range/8);

    vector<Point> dataset(n);

    gen_datapoints(dataset, 0      , n/4         , min_distr, min_distr);
    gen_datapoints(dataset, n/4    , n/2         , min_distr, max_distr);
    gen_datapoints(dataset, n/2    , (n*3)/4     , max_distr, min_distr);
    gen_datapoints(dataset, (n*3)/4, n           , max_distr, max_distr);

    // cout << "Dataset:" << endl;
    // print_dataset(dataset);

    vector<Point> queries;
    vector<vector<uint32_t>> answers;
    gen_datasets(dataset, queries, answers);

    // cout << "Queries:" << endl;
    // print_dataset(queries);


    return 0;
}