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

//compile with g++ arreader.cpp -o arreader -O3 -Wall -std=c++11 -march=native -I ../external/simple-serializer -I ../external/eigen -lboost_program_options
//run with ./arreader -i [input folder] -s [subset divisor]
int main(int argc, char **argv)
{
    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "print usage message")("input,i", po::value<string>(), "input dataset")("size,s", po::value<int>(), "divisor of dataset");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    string input_folder = vm["input"].as<string>();
    int sub_size = vm["size"].as<int>();
    cout << "Parameters registered: Input folder = '" << input_folder << "' and subset divisor: " << sub_size << endl; 

    vector<Point> dataset;
    deserialize(input_folder + "/dataset.dat", &dataset);
    int n = dataset.size();
    cout << "subset size = " << (n/sub_size) << endl;
    double lowest = 1e100, highest = -1e100;
  
    for(int i = 0; i< (n/sub_size); ++i) {
        for (int j = i+1; j < (n/sub_size); ++j)
        {
            double score = (dataset[i] - dataset[j]).norm();
            if (score < lowest)
            {
                if(score != 0) lowest = score;
                else cout << score;
            }
            if (score > highest)
            {
                highest = score;
            }
        }
    }
    cout << "highest: " << highest << endl; 
    cout << " lowest: " << lowest << endl; 

    double a_r = highest/lowest;

    cout << "ar: " << a_r << endl;
}