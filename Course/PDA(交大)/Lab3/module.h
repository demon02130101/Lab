#ifndef MODULE_H
#define MODULE_H

using namespace std;

class Cell
{
public:
    Cell() {}
    Cell(string n, int x_, int y_, int w_, int h_, bool fix)
        : name(n), x(x_), y(y_), w(w_), h(h_), isFix(fix) {}
    ~Cell() {}

    string name;
    int x, y;
    int w, h;
    bool isFix;
};

class Row
{
public:
    Row() {}
    ~Row() {}

    int x, y;
    int site_w, site_h;
    int num_site;
};

class Segment
{
public:
    Segment() {}
    ~Segment() {}

    int x, y;
    int site_w, site_h;
    int num_site;
};
#endif