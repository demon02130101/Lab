#ifndef LG_H
#define LG_H
#include <vector>
#include <fstream>
#include <unordered_map>
#include "module.h"

using namespace std;

class Legalizer
{
public:
    Legalizer() {}
    ~Legalizer() {}

    double alpha, beta;
    int minX, minY, maxX, maxY;
    int Rx, Ry; // window maintain
    int row_h;
    int row_sites;
    int next_x;
    string output_name;
    vector<Cell *> cells;
    unordered_map<string, Cell *> name2cells;
    vector<Row *> rows;

    // I/O
    void Read_lg(ifstream &input);
    void Run_opt(ifstream &input);
    void Post_lg(ofstream &output);

    // local legalization
    bool MLL(Cell *FF, int x, int y);
    void ClampToBounds(int &pos, int size, int minBound, int maxBound);
    bool isTouchW(Cell &a, Window &W);
    bool isTouchFF(Cell &a, Cell &b);
    bool isInW(Cell *a, Window *W);
    bool isOverlap(Cell &a, Cell &b);
    Point FindBestPoint(vector<Point> insert_points, Cell &mFF);
    void FindGaps(const vector<vector<Cell>> &W_cells, vector<vector<Gap>> &gaps, const Window &W, const Cell &tFF);

    // calulation
   
    int Randx(int k);
    int Randy(int k);
    static bool compareX(Cell &a, Cell &b);
    static bool CompareX(Cell *a, Cell *b);

    // print
    void printCells(const vector<Cell *> &cells);
    void printRows(const vector<Row *> &rows);
    void printRow_cells();
};
#endif
