#include <algorithm>
#include "Network.h"
// I/O
void Network::ReadFile(ifstream &input)
{
    string line, name;
    if (getline(input, line))
    {
        istringstream ss(line);
        ss >> name >> N >> I >> O;
    }
    // initialize
    adj = new list<int>[N];
    rev_adj = new list<int>[N];
    fanin_list = new vector<int>[N];
    LUT = new unordered_set<int>[N];
    isPI = new bool[N];
    isPO = new bool[N];

    for (int i = 0; i <= N; i++)
    {
        isPI[i] = false;
        isPO[i] = false;
    }
    // Read the IDs of primary inputs
    for (int i = 0; i < I; i++)
    {
        if (getline(input, line))
        {
            int pi_id = stoi(line); //
            isPI[pi_id - 1] = true;
            // Node *node = new Node(pi_id);        //
            // node_list[pi_id] = node; //
        }
    }
    // Read the IDs of primary outputs
    for (int i = 0; i < O; i++)
    {
        if (getline(input, line))
        {
            int po_id = stoi(line);
            isPO[po_id - 1] = true;
        }
    }
    // Read the ID of the node and its inputs
    for (int i = 0; i < N - I; i++)
    {
        if (getline(input, line))
        {
            istringstream ss(line);
            int node_id, fanin_id;
            ss >> node_id;

            // Node *node = new Node(node_id); //
            while (ss >> fanin_id)
            {
                // node->input_list.push_back(input_id); //
                fanin_list[node_id - 1].push_back(fanin_id - 1);
                adj[fanin_id - 1].push_back(node_id - 1);
                rev_adj[node_id - 1].push_back(fanin_id - 1);
            }
            // node_list[node_id] = node; //
        }
    }
    /*
    for (int i = 0; i < N; i++)
    {
        cout << "Node " << i + 1 << ": ";
        for (int neighbor : adj[i])
        {
            cout << neighbor + 1 << " ";
        }
        cout << endl;
    }
    */
    input.close();
}

void Network::OutputFile(ofstream &output)
{
    vector<int> visited(0, 0);
    std::queue<int> map_q;
    for (int i = 0; i < N; i++)
    {
        if (isPO[i])
        {
            map_q.push(i);
            visited.push_back(i);
        }
    }

    while (map_q.empty() == false)
    {
        int ID = map_q.front();
        map_q.pop();
        if (find(visited.begin(), visited.end(), ID) == visited.end())
            visited.push_back(ID);

        for (auto it = LUT[ID].begin(); it != LUT[ID].end(); it++)
        {
            int LUT_node = *it;
            // cout << "lut: " << LUT_node + 1 << endl;
            // cout << "fanin: ";
            for (auto it2 = fanin_list[LUT_node].begin(); it2 != fanin_list[LUT_node].end(); it2++)
            {
                int fanin_node = *it2;
                // cout << fanin_node + 1 << " ";

                bool OK = true;
                for (auto it3 = LUT[ID].begin(); it3 != LUT[ID].end(); it3++)
                {
                    if (*it3 == fanin_node)
                        OK = false;
                }

                if (OK && find(visited.begin(), visited.end(), fanin_node) == visited.end() && !isPI[fanin_node]) // check for duplication & node in LUT list
                    map_q.push(fanin_node);
            }
            // cout << endl;
        }
    }

    vector<vector<int>> result;
    result.resize(visited.size());

    for (int i = 0; i < visited.size(); i++)
    {
        int ID = visited[i];

        for (auto itr = LUT[ID].begin(); itr != LUT[ID].end(); itr++)
        {
            int LUT_node = *itr;
            // cout << "lut: " << LUT_node + 1 << endl;
            // cout << "fanin: ";
            for (auto itr2 = fanin_list[LUT_node].begin(); itr2 != fanin_list[LUT_node].end(); itr2++)
            {
                int fanin_node = *itr2;
                // cout << fanin_node + 1 << " ";
                bool OK = true;
                for (auto itr3 = LUT[ID].begin(); itr3 != LUT[ID].end(); itr3++)
                {
                    if (*itr3 == fanin_node)
                        OK = false;
                }
                if (OK && find(result[i].begin(), result[i].end(), fanin_node) == result[i].end())
                    result[i].push_back(fanin_node);
            }
            // cout << endl;
        }
        result[i].push_back(ID);
    }

    // OutFile
    for (int i = 0; i < result.size(); i++)
    {
        for (int j = result[i].size() - 1; j >= 0; j--)
            output << result[i][j] + 1 << " ";
        output << endl;
    }
    output.close();
}

// mapping
void Network::MapTree()
{
    while (topo_Stack.empty() == false)
    {
        int curr_node = topo_Stack.top();
        /*
        cout << "Node:" << curr_node + 1 << endl;
        cout << "fanin:";
        for (auto i = fanin_list[curr_node].begin(); i != fanin_list[curr_node].end(); i++)
        {
            int fanin_node = *i + 1;
            cout << fanin_node << " ";
        }
        cout << endl;
        */
        Sort_Fanins(curr_node);
        /*
        cout << "fanin_sort:";
        for (auto i = fanin_list[curr_node].begin(); i != fanin_list[curr_node].end(); i++)
        {
            int fanin_node = *i + 1;
            cout << fanin_node << " ";
        }
        cout << endl;
        */

        // cout << "curr_node = " << curr_node + 1 << endl;
        // Greedy packing
        int total_fanin_size = 0;
        unordered_set<int> added_fanins; // 用來紀錄已經計算的 fanin
        t_LUT = new unordered_set<int>[N];

        LUT[curr_node].insert(curr_node);
        t_LUT[curr_node].insert(curr_node);
        for (int wi : fanin_list[curr_node])
        {
            // cout << "  |input(LUT[" << wi + 1 << "])| = " << GetLUTFaninSize(wi, LUT) << endl;
            t_LUT[curr_node].insert(curr_node);
            // 先加入LUT[wi]看看
            for (int it : LUT[wi])
            {
                t_LUT[curr_node].insert(it);
                //cout << "  try add " << it + 1 << " to " << "LUT[" << curr_node + 1 << "]" << endl;
            }

            // K-feasible?
            // if yes, add to LUT[curr_node]
            if (GetLUTFaninSize(curr_node, t_LUT) <= K)
            {
                for (int it : LUT[wi])
                {
                    LUT[curr_node].insert(it);
                    // cout << "  add " << it + 1 << " to " << "LUT[" << curr_node + 1 << "]" << endl;
                }
                total_fanin_size = GetLUTFaninSize(curr_node, LUT);
                // cout << "  LUT_size = " << total_fanin_size << endl;
            }
            else // if no, not add to LUTi and clear t_LUTi
            {
                t_LUT[curr_node].clear();
            }
        }
        // PrintLUT(curr_node);
        topo_Stack.pop();
    }
}

void Network::Sort_Fanins(int curr_node)
{
    // 自定義比較函數，根據 LUT[xi] 的 fanin 大小進行比較
    auto compare = [this](int a, int b)
    {
        return GetLUTFaninSize(a, LUT) < GetLUTFaninSize(b, LUT);
    };

    // 對 fanin_list[curr_node] 進行排序
    sort(fanin_list[curr_node].begin(), fanin_list[curr_node].end(), compare);
}

int Network::GetLUTFaninSize(int node, unordered_set<int> *temp_LUT)
{
    // int total_fanin_size = 0;

    // 建立一個 set，存儲 LUT[xi] 中的所有節點，方便快速查找
    unordered_set<int> lut_set(temp_LUT[node].begin(), temp_LUT[node].end());
    unordered_set<int> fanin_set;

    // 遍歷 LUT[xi] 中的所有節點
    for (int it : temp_LUT[node])
    {
        // 遍歷該節點的 fanin_list
        for (int fanin : fanin_list[it])
        {
            // 如果 fanin 不在 LUT[xi] 中，則累加
            if (lut_set.find(fanin) == lut_set.end())
            {
                // total_fanin_size++;
                fanin_set.insert(fanin);
            }
        }
    }
    return fanin_set.size();
}
// topological
void Network::topologicalSort()
{
    // Mark all the vertices as not visited
    bool *visited = new bool[N];
    for (int i = 0; i < N; i++)
        visited[i] = false;

    // Call the recursive helper function to store Topological
    // Sort starting from all vertices one by one
    for (int i = 0; i < N; i++)
        if (visited[i] == false)
            topologicalSortUtil(i, visited, topo_Stack);

    // Print contents of stack
    /*
    while (topo_Stack.empty() == false)
    {
        cout << topo_Stack.top() + 1 << " ";
        topo_Stack.pop();
    }
    */
}

void Network::topologicalSortUtil(int v, bool visited[], stack<int> &topo_Stack)
{
    // Mark the current node as visited.
    visited[v] = true;
    // Recur for all the vertices adjacent to this vertex
    list<int>::iterator i;
    for (i = adj[v].begin(); i != adj[v].end(); i++)
        if (!visited[*i])
            topologicalSortUtil(*i, visited, topo_Stack);

    // Push current vertex to stack which stores result
    if (!isPI[v])
        topo_Stack.push(v);
}

void Network::addEdge(int i, int j)
{
    adj[i].push_back(j); // Add node j to node i’s list.
}

// Debug
void Network::PrintLUT(int xi)
{
    cout << "LUT[" << xi + 1 << "] = { ";
    for (const auto &element : LUT[xi])
    {
        cout << element + 1 << " ";
    }
    cout << "}" << endl;
}