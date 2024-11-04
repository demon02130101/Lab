#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <chrono>
#include "Network.h"
using namespace std;

// Main function to process input and execute mapping
int main(int argc, char *argv[])
{
    ifstream input;
    ofstream output;
    Network mp;
    if (argc != 4)
    {
        cerr << "Please set the ./mapper <input file path> <output file path> <K>!" << endl;
        return 1;
    }
    else if (argc == 4)
    {
        input.open(argv[1]);
        output.open(argv[2]);
        mp.K = stod(argv[3]);
        if (!input)
        {
            cerr << argv[1] << " failed to open!" << endl;
            return 1;
        }
        if (!output)
        {
            cerr << argv[2] << " failed to open!" << endl;
            return 1;
        }
    }
    auto start = chrono::high_resolution_clock::now();

    mp.ReadFile(input);
    mp.topologicalSort();
    mp.MapTree();
    mp.OutputFile(output);

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> runtime = end - start;
    // cout << "runtime = " << runtime.count();
    return 0;
}
