// normal_distribution
#include <iostream>
#include <string>
#include <random>
#include <boost/program_options.hpp>
#include "serialize.h"
#include <chrono>

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

int check_input(int argc, char **argv)
{
    //Description of arguments
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "print usage message")
        ("nodes,n", po::value<int>(), "number of nodes")
        ("dimensions,d", po::value<int>(), "number of dimensions")
        ("min,i", po::value<float>(), "min value")
        ("max,a", po::value<float>(), "max value");
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
    if (!vm.count("nodes") || !vm.count("dimensions") || !vm.count("min") || !vm.count("max"))
    {
        cout << desc << endl;
        throw runtime_error("number of nodes and dimensions, and min and max values must be specified");
    }
    n = vm["nodes"].as<int>();
    d = vm["dimensions"].as<int>();
    min_value = vm["min"].as<float>();
    max_value = vm["max"].as<float>();
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

    for(int i = 0; i<n; ++i)
    {
        for(int j = 0; j<d; ++j)
        {
            cout << dataset[i][j] << ", " << flush;
        }
        cout << endl;
    }

    // const int nrolls = 10000; // number of experiments
    // const int nstars = 100;   // maximum number of stars to distribute

    // default_random_engine generator;
    // normal_distribution<double> distribution(5.0, 2.0);

    // int p[10] = {};

    // for (int i = 0; i < nrolls; ++i)
    // {
    //     double number = distribution(generator);
    //     if ((number >= 0.0) && (number < 10.0))
    //         ++p[int(number)];
    // }

    // std::cout << "normal_distribution (5.0,2.0):" << std::endl;

    // for (int i = 0; i < 10; ++i)
    // {
    //     std::cout << i << "-" << (i + 1) << ": ";
    //     std::cout << std::string(p[i] * nstars / nrolls, '*') << std::endl;
    // }

    return 0;
}