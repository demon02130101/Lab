#ifndef FLOW_H_
#define FLOW_H_

// C++ code
#include <iostream>
#include <vector>
#include <queue>

using namespace std;
class Graph_FlowNetWorks
{
private:
    int num_vertex;
    std::vector<std::vector<int>> AdjMatrix;
    int maxflow = 0;

public:
    Graph_FlowNetWorks() : num_vertex(0) {};
    Graph_FlowNetWorks(int n);
    void AddEdge(int from, int to, int capacity);

    std::vector<int> FordFulkerson(int source, int termination);
    bool BFSfindExistingPath(std::vector<std::vector<int>> graphResidual, int *predecessor, int source, int termination);
    int MinCapacity(std::vector<std::vector<int>> graphResidual, int *predecessor, int termination);
    std::vector<bool> BFS(std::vector<std::vector<int>> matrix, int start);
    int get_maxflow() { return maxflow; };
};

#endif
