#include "bstar.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <math.h>
#include <random>
#include <pthread.h>
#include <thread>
#include <mutex>
#include <limits>
#include <memory> // for std::unique_ptr

using namespace std;

mutex mtx;

struct ThreadData
{
    unique_ptr<Floorplanner> fp;
    unique_ptr<string> block_filename;
    unique_ptr<string> net_filename;
};

void run_floorplanner(ThreadData *data)
{
    ifstream input_block(*data->block_filename);
    ifstream input_net(*data->net_filename);

    if (!input_block || !input_net)
    {
        cerr << "Error opening input files." << endl;
        return;
    }

    data->fp->ReadBlocks(input_block);
    data->fp->ReadNets(input_net);
    data->fp->SA();
}

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

    // allocate multithread
    vector<unique_ptr<Floorplanner>> fps(6);
    vector<thread> threads(6);
    vector<ThreadData> thread_data(6);

    for (int i = 0; i < 6; i++)
    {
        fps[i] = make_unique<Floorplanner>();
        fps[i]->alpha = alpha;

        thread_data[i] = {move(fps[i]), make_unique<string>(argv[2]), make_unique<string>(argv[3])};
        threads[i] = thread(run_floorplanner, &thread_data[i]); // use std::thread instead of pthread_create
    }

    // wait for completion
    for (auto &t : threads)
    {
        if (t.joinable())
            t.join(); // join threads
    }
    // find best case
    unique_ptr<Floorplanner> best_fp = move(thread_data[0].fp);
    double best_trcost = best_fp->best_cost;

    for (int i = 1; i < 6; i++)
    {
        if (thread_data[i].fp && thread_data[i].fp->best_cost < best_trcost)
        {
            //lock_guard<mutex> lock(mtx); // use std::lock_guard to lock the mutex
            best_fp = move(thread_data[i].fp);
            best_trcost = best_fp->best_cost;
        }
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> runtime = end - start;

    best_fp->OutputFile(output, runtime);

    /*
    auto start = chrono::high_resolution_clock::now();

    Floorplanner fp;

    fp.alpha = alpha;
    fp.ReadBlocks(input_block);
    fp.ReadNets(input_net);
    fp.All_blkArea();
    fp.SA(); // init_t, end_t, cooling_rate, iter

    // fp.Cal_Tsteps(10000000, 0.00001, 0.8); // init_t, end_t, cooling_rate
    // int num_threads = 8;
    // int iter_per_thread = 1000;
    // fp.SA(num_threads, iter_per_thread);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> runtime = end - start;
    */
    return 0;
}