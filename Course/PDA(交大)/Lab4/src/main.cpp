#include <iostream>
#include <fstream>
#include "GR.h"
#include "module.h"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        cerr << "Please set the <D2DGRter> <*.gmp> <*.gcl> <*.cst> <*.lg>!" << endl;
        return 1;
    }

    GR *gr = new GR(argv[1], argv[2], argv[3], argv[4]);

    return 0;
}