#include "bstar.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <math.h>
#include <random>
#include <thread>
using namespace std;

int main(int argc, char *argv[])
{
    ifstream input_block, input_net;
    ofstream output;
    double alpha;
    if (argc != 5)
    {
        cerr << "Please set the [alpha value] [input.block name] [input.net name] [output name]!" << endl;
        return 1;
    }
    else if (argc == 5)
    {
        alpha = stod(argv[1]); // access alpha value
        input_block.open(argv[2]);
        input_net.open(argv[3]);
        output.open(argv[4]);
        if (alpha < 0.0 || alpha > 1.0)
        {
            cerr << "Alpha must be between 0 and 1." << endl;
            return 1;
        }
        if (!input_block)
        {
            cerr << argv[2] << " failed to open!" << endl;
            return 1;
        }
        if (!input_net)
        {
            cerr << argv[3] << " failed to open!" << endl;
            return 1;
        }
        if (!output)
        {
            cerr << argv[4] << " failed to open!" << endl;
            return 1;
        }
    }
    auto start = chrono::high_resolution_clock::now();

    Floorplanner fp;
    
    fp.alpha = alpha;
    fp.ReadBlocks(input_block);
    fp.ReadNets(input_net);
    fp.SA(); // init_t, end_t, cooling_rate, iter
    //fp.All_blkArea();
    //fp.Cal_Tsteps(10000000, 0.00001, 0.8); // init_t, end_t, cooling_rate
    //int num_threads = 8; 
    //int iter_per_thread = 1000;
    //fp.SA(num_threads, iter_per_thread);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> runtime = end - start;

    fp.OutputFile(output, runtime);
    return 0;
}