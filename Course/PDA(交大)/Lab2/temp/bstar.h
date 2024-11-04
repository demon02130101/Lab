#ifndef BSTAR_H
#define BSTAR_H
#include "module.h"
#include <vector>
#include <map>
#include <string>
#include <chrono>
using namespace std;

class Floorplanner
{
public:
    Floorplanner() = default;
    ~Floorplanner() {}
    // variables
    double alpha;
    double deltaAvg;
    double avg_area, avg_hpwl;
    int outline_width, outline_height;
    int num_blk, num_tml, num_net;
    int x_max, y_max;
    int best_xmax, best_ymax;
    double best_cost, best_area, best_hpwl;
    // I/O
    void ReadBlocks(ifstream &input);
    void ReadNets(ifstream &input);
    void OutputFile(ofstream &output, const chrono::duration<double> &runtime);
    // floorplanning
    void SA();
    void BuildTree();
    void DelTree(Node *node);
    void DelTree();
    void Upd_Sol();
    //void SaveBestTree(Node *node);
    void SetPos(Node *node);
    int SetContour_Caly(Node *node);
    // Pertubation
    void Rotate_Node(int node); // not yet
    void Move_Node(int from_mode, int to_node);
    void Swap_Node(int node1, int node2);
    // calculation
    double Cal_Cost();
    double Cost();
    int Cal_HPWL();
    void Cal_Avg();
    void Cal_deltaAvg();
    // Debug
    void All_blkArea();
    void Cal_Tsteps(double T, double end_T, double rate);

private:
    Node *root;
    vector<Contour> contour_list;
    vector<Block *> blk_list;
    vector<Terminal *> tml_list;
    vector<Node *> node_list;
    vector<Net *> net_list;
    // easiler to get the info via name
    map<string, Block *> blk_map;
    map<string, Terminal *> tml_map;
    map<string, Node *> node_map;
};
#endif