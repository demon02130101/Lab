#ifndef NETWORK_H
#define NETWORK_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <stack>
#include <limits>
using namespace std;

// Node structure
class Node
{
public:
    Node() {}
    Node(int id_) : id(id_) {}
    ~Node() {}

    int id;
    // vector<int> input_list;
};

class Network
{
public:
    Network() {}
    ~Network() {}

    int K, N, I, O;                       // N: no. of gates in boolean function, I: no. of primary input, O: no. of primary output
    unordered_map<int, Node *> node_list; // boolean network, ID to Node
    // I/O
    void ReadFile(ifstream &input);
    void OutputFile(ofstream &output);
    // mapping
    void MapTree();

    // Topological sort
    void addEdge(int v, int w); // function to add an edge to graph
    void topologicalSort();     // prints a Topological Sort of the complete graph
    void topologicalSortUtil(int v, bool visited[], stack<int> &Stack);

private:
    bool *isPI, *isPO;
    int *opt_area;
    list<int> *adj, *rev_adj;
    unordered_set<int> *LUT, *t_LUT;
    vector<int> *fanin_list;
    stack<int> topo_Stack;

    // mapping
    void Sort_Fanins(int curr_node);
    int GetLUTFaninSize(int node, unordered_set<int> *LUT);

    // Debug
    void PrintLUT(int node);
};

#endif