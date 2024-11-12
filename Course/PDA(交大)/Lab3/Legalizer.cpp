#include "module.h"
#include "lg.h"
#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char *argv[])
{
    ifstream lg_file, opt_file;
    ofstream output;
    if (argc != 4)
    {
        cerr << "Please set the <Legalizer> <*.lg> <*.opt> <*_post.lg>!" << endl;
        return 1;
    }
    else if (argc == 4)
    {
        lg_file.open(argv[1]);
        opt_file.open(argv[2]);
        output.open(argv[3]);
        if (!lg_file)
        {
            cerr << argv[1] << " failed to open!" << endl;
            return 1;
        }
        if (!opt_file)
        {
            cerr << argv[2] << " failed to open!" << endl;
            return 1;
        }
        if (!output)
        {
            cerr << argv[3] << " failed to open!" << endl;
            return 1;
        }
    }
}