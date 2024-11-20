#ifndef MODULE_H
#define MODULE_H
#include <iostream>
#include <string>
#include <vector>
using namespace std;

class Cell
{
public:
    Cell() {}
    Cell(string n, int x_, int y_, int w_, int h_, bool fix)
        : name(n), x(x_), y(y_), w(w_), h(h_), isFix(fix) {}
    ~Cell() {}

    string name;
    long x, y;
    long w, h;
    bool isFix;
};

class Site
{
public:
    Site() {}
    Site(int x_, int y_, int w_, int h_)
        : x(x_), y(y_), w(w_), h(h_) {}
    ~Site() {}

    int x, y;
    int w, h;
};

class Segment
{
public:
    Segment() {}
    Segment(int x_, int y_, int site_w_, int site_h_, int num_site_)
        : x(x_), y(y_), site_w(site_w_), site_h(site_h_), num_site(num_site_) {}
    ~Segment() {}

    int x, y;
    int site_w, site_h;
    int num_site;
    vector<Site *> sites;
    vector<Cell *> cells;
};

class Row
{
public:
    Row() {}
    Row(int x_, int y_, int w_, int h_, int num_site_)
        : x(x_), y(y_), site_w(w_), site_h(h_), num_site(num_site_) {}
    ~Row() {}

    int x, y;
    int site_w, site_h;
    int num_site;
    vector<Site *> sites;
    vector<Segment *> segments;
    vector<Cell *> cells;
};

class Window
{
public:
    Window() {}
    Window(int x_, int y_, int w_, int h_)
        : x(x_), y(y_), w(w_), h(h_) {}
    ~Window() {}

    int x, y;
    int w, h;

    vector<Segment *> loc_segments;
};

class Point
{
public:
    Point() {}
    Point(int x_, int y_)
        : x(x_), y(y_) {}
    ~Point() {}

    int x, y;
};

class Gap
{
public:
    Gap() {}
    Gap(int xL_, int xR_)
        : xL(xL_), xR(xR_), length(xR_ - xL_) {}
    ~Gap() {}

    int xL, xR, length;
};
#endif