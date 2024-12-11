#ifndef MODULE_H
#define MODULE_H
#include <iostream>
#include <string>
#include <vector>

using namespace std;
enum Dir
{
    M1,
    M2
}; // vertical horizontal
class Gcell
{
public:
    Gcell() : num_left(0), num_bottom(0) {}
    ~Gcell() {}

    int x, y;
    int id_y, id_x;
    int left_cap, bottom_cap;
    int num_left, num_bottom;
    double M1_cost, M2_cost; // M1:vertical M2:horizontal cost
};

class Node
{
public:
    Node() {}
    ~Node() {}

    Node *parent = nullptr;
    int x, y;
    int id_x, id_y;
    int left_cap, bottom_cap;
    int num_left, num_bottom;
    double M1_cost, M2_cost; // M1:vertical M2:horizontal cost

    double f_cost, g_cost, h_cost;
    Dir dir, temp_dir; // 水平或垂直方向被找到的

    bool operator==(const Node &other) const
    {
        return id_x == other.id_x && id_y == other.id_y;
    }
};

class Bump
{
public:
    Bump() {}
    ~Bump() {}

    int id;
    int x, y;
    vector<Node> net;
};

#endif