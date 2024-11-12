#ifndef LG_H
#define LG_H
#include <vector>
#include <fstream>
#include "module.h"

using namespace std;

class Legalizer
{
public:
    Legalizer() {}
    ~Legalizer() {}

    double alpha, beta;
    int minX, minY, maxX, maxY;
    vector<Cell *> cells;
    vector<Cell *> unplaced_FFs; // temp
    vector<Row *> rows;

    // I/O
    void ReadFile(ifstream &input);
    void OutputFile(ofstream &output);

    // calulation
    int Eval_Cost();
};
#endif

