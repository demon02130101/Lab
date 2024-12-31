#ifndef GR_H
#define GR_H
#include <vector>
#include <unordered_set>
#include "module.h"

using namespace std;

class GR
{
public:
    GR() {}
    GR(string gmp_file, string gcl_file, string cst_file, string output_file);
    ~GR() {}

    // Variables
    int area_x, area_y, area_w, area_h;
    int chip1_x, chip1_y, chip1_w, chip1_h;
    int chip2_x, chip2_y, chip2_w, chip2_h;
    int grid_w, grid_h;
    int num_col; // area_width / grid_width
    int num_row; // area_height / grid_height
    double alpha, beta, gamma, delta;
    double via_cost;
    vector<vector<Gcell>> Gcells;
    vector<Bump> s_bumps;
    vector<Bump> t_bumps;
    // vector<vector<double>> M1_cost;
    // vector<vector<double>> M2_cost;

    // I/O
    void read_gmp(string gmp_file);
    void read_gcl(string gcl_file);
    void read_cst(string cst_file);
    void write_ans(string output_file);

    // route
    void Run();
    void A_star(Bump &s_bump, Bump &t_bump);
    void init_nodes(const vector<vector<Gcell>> &Gcells, vector<vector<Node>> &nodes);
    // Node select_node(vector<Node> open_set);
    Node *select_node(vector<Node *> &open_set, Node *t_node);
    void construct_net(Bump &s_bump, Node *cur_node);
    double cal_g(Node *cur_node, Node *next_node);
    double cal_h(Node *t_node, Node *next_node);
};

#endif