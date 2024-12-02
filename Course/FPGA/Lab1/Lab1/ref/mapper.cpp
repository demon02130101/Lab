#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <sstream>
#include <list>
#include <stack>
#include <queue>
#include <algorithm>

#include "Graph_FlowNetWorks.h"

using namespace std;

#define INF 10000

// register funtions
void readFile(const string infile_name);
void topologicalSort();
void topologicalSortUtil(int v, bool visited[], stack<int> &topo_Stack);
int *con_network(int Start);
void labeling();
void delete_mem();
void printLabel();
void mapping(const string outfile_name);

// global variable
int N = 0, I = 0, O = 0, K = 0;
list<int> *adj = NULL, *rev_adj = NULL, *kLUT_list = NULL;
bool *isPI = NULL, *isPO = NULL;
int *label = NULL;
stack<int> topo_Stack;

int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		cerr << "Please set the ./mapper <input file path> <output file path> <K>!" << endl;
		return 1;
	}
	// Preprocessing
	readFile(argv[1]);
	K = stoi(argv[3]);
	/*
	== Labeling Phase ==
	*/
	topologicalSort();
	labeling();
	// printLabel();
	/*
	== Mapping Phase ==
	*/
	mapping(argv[2]);
	// Postprocessing
	delete_mem();
	return 0;
}

void mapping(const string outfile_name)
{
	vector<int> visited(0, 0);
	std::queue<int> map_q;
	for (int i = 0; i < N; i++)
	{
		if (isPO[i])
		{
			map_q.push(i + 1);
			visited.push_back(i + 1);
		}
	}

	while (map_q.empty() == false)
	{
		int ID = map_q.front();
		map_q.pop();
		if (find(visited.begin(), visited.end(), ID) == visited.end())
			visited.push_back(ID);

		for (list<int>::iterator itr = kLUT_list[ID].begin(); itr != kLUT_list[ID].end(); itr++)
		{
			int lut_node = *itr;
			// cout << "lut: " << lut_node << endl;
			// cout << "fanin: ";
			for (list<int>::iterator k = rev_adj[lut_node - 1].begin(); k != rev_adj[lut_node - 1].end(); k++)
			{
				int fanin_node = *k + 1;
				// cout << fanin_node << " ";

				bool OK = true;
				for (list<int>::iterator itr1 = kLUT_list[ID].begin(); itr1 != kLUT_list[ID].end(); itr1++)
				{
					if (*itr1 == fanin_node)
						OK = false;
				}

				if (OK && find(visited.begin(), visited.end(), fanin_node) == visited.end() && !isPI[fanin_node - 1]) // check for duplication & node in LUT list
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

		for (list<int>::iterator itr = kLUT_list[ID].begin(); itr != kLUT_list[ID].end(); itr++)
		{
			int lut_node = *itr;
			cout << "lut: " << lut_node << endl;
			cout << "fanin: ";
			for (list<int>::iterator k = rev_adj[lut_node - 1].begin(); k != rev_adj[lut_node - 1].end(); k++)
			{
				int fanin_node = *k + 1;
				cout << fanin_node << " ";
				bool OK = true;
				for (list<int>::iterator itr1 = kLUT_list[ID].begin(); itr1 != kLUT_list[ID].end(); itr1++)
				{
					if (*itr1 == fanin_node)
						OK = false;
				}
				if (OK && find(result[i].begin(), result[i].end(), fanin_node) == result[i].end())
				{ // check for duplication & node in LUT list
					result[i].push_back(fanin_node);
				}
			}
			cout << endl;
		}
		result[i].push_back(ID);
	}

	cout << "result size " << result.size() << endl;

	// write to output file

	ofstream fp;
	fp.open(outfile_name.c_str());
	for (int i = 0; i < result.size(); i++)
	{
		for (int j = result[i].size() - 1; j >= 0; j--)
			fp << result[i][j] << " ";
		fp << endl;
	}
	fp.close();
}

void printLabel()
{
	cout << "final labels:" << endl;
	for (int i = 0; i < N; i++)
	{
		cout << label[i] << " ";
	}
	cout << endl;
}

void labeling()
{
	// initializing label array to -1, PI to 0
	for (int i = 0; i < N; i++)
	{
		if (isPI[i])
			label[i] = 0;
		else
			label[i] = -1;
	}

	// by topological order, ommit one node for connecting network

	while (topo_Stack.empty() == false)
	{
		int p = 0; // temp label
		int current_node = topo_Stack.top();
		int *pred_ret;
		pred_ret = con_network(current_node); // 有無predecessor of the current node

		// find p by searching all parent of current node
		for (int i = 0; i < N; i++)
		{
			if (pred_ret[i] >= 0)
			{
				p = (p < label[i]) ? label[i] : p;
			}
		}

		// Nt -> Nt'
		for (int i = 0; i < N; i++)
		{
			if (p == label[i] && pred_ret[i] >= 0)
				pred_ret[i] = -10;
		}

		int num_merge = 0;
		bool set = false;
		for (int i = 0; i < N; i++)
		{
			if (pred_ret[i] == -10 || pred_ret[i] > 0)
			{
				if (pred_ret[i] == -10 && set == false)
				{
					num_merge++;
					set = true;
				}
				else if (pred_ret[i] > 0)
				{
					num_merge++;
				}
			}
		}

		// cout << "num of nodes after merging: "<< num_merge << endl;

		int num_Ntp_node = num_merge + 1;
		int *Nt_p = new int[num_Ntp_node * num_Ntp_node]; // including s(0)
		int *Nt_p_index = new int[num_Ntp_node];

		// initialization
		for (int i = 0; i < num_Ntp_node; i++)
		{
			for (int j = 0; j < num_Ntp_node; j++)
			{
				Nt_p[i * num_Ntp_node + j] = 0;
			}
		}

		int offset = 1;
		Nt_p_index[0] = 0; // s
		// construct index array
		for (int i = 0; i < N; i++)
		{
			if (pred_ret[i] >= 0)
			{
				Nt_p_index[offset] = i + 1;
				offset++;
			}
			else if (pred_ret[i] == -10 && current_node == i)
			{
				Nt_p_index[offset] = current_node + 1;
				offset++;
			}
		}

		// fill in Nt_p
		for (int i = 0; i < N; i++)
		{
			if (pred_ret[i] == -10)
			{ // node's fanin will be assigned to node t'
				for (int j = 0; j < num_Ntp_node; j++)
				{
					if (Nt_p_index[j] == current_node + 1)
					{
						if (isPI[i])
						{
							Nt_p[num_Ntp_node * 0 + j] = 1; // s to Nt_p
						}
						else
						{
							list<int>::iterator k;
							for (k = rev_adj[i].begin(); k != rev_adj[i].end(); ++k)
							{
								if (pred_ret[*k] != -10 && pred_ret[*k] >= 0)
								{
									for (int l = 0; l < num_Ntp_node; l++)
									{
										if (Nt_p_index[l] == *k + 1)
										{
											Nt_p[num_Ntp_node * (l) + j] = 1;
										}
									}
								}
							}
						}
					}
				}
			}
			else if (pred_ret[i] >= 0)
			{ // node's fanin will be
				for (int j = 0; j < num_Ntp_node; j++)
				{
					if (Nt_p_index[j] == i + 1)
					{
						if (isPI[i])
						{
							Nt_p[num_Ntp_node * 0 + j] = 1;
						}
						else
						{
							list<int>::iterator k;
							for (k = rev_adj[i].begin(); k != rev_adj[i].end(); ++k)
							{
								if (pred_ret[*k] != -10 && pred_ret[*k] >= 0)
								{
									for (int l = 0; l < num_Ntp_node; l++)
									{
										if (Nt_p_index[l] == *k + 1)
										{
											Nt_p[num_Ntp_node * (l) + j] = 1;
										}
									}
								}
							}
						}
					}
				}
			}
		}

		// Nt' -> Nt''
		int num_mid = num_merge - 1;
		int Nt_pp_num_node = 2 * num_mid + 2;
		int *Nt_pp = new int[Nt_pp_num_node * Nt_pp_num_node]; // including s(0)
		int *Nt_pp_index = new int[Nt_pp_num_node];
		Graph_FlowNetWorks g11(Nt_pp_num_node);

		for (int i = 0; i < Nt_pp_num_node; i++)
		{
			for (int j = 0; j < Nt_pp_num_node; j++)
			{
				Nt_pp[i * Nt_pp_num_node + j] = 0;
			}
		}

		offset = 1;
		Nt_pp_index[0] = 0;									// s(PI)
		Nt_pp_index[Nt_pp_num_node - 1] = current_node + 1; // PO

		// construct index array
		for (int i = 0; i < (Nt_pp_num_node) / 2 - 1; i++)
		{
			Nt_pp_index[2 * offset - 1] = Nt_p_index[offset];
			Nt_pp_index[2 * offset] = -Nt_p_index[offset];
			offset++;
		}

		for (int i = 0; i < num_merge; i++)
		{
			for (int j = 0; j < num_merge + 1; j++)
			{
				if (Nt_p[i * (num_merge + 1) + j] && i == 0)
				{
					int to = 2 * j - 1;
					g11.AddEdge(0, to, INF);
					Nt_pp[0 * Nt_pp_num_node + (to)] = INF;
				}
				else if (Nt_p[i * (num_merge + 1) + j])
				{
					int from = 2 * i - 1;
					int mid = 2 * i;
					int to = 2 * j - 1;
					g11.AddEdge(from, mid, 1);
					g11.AddEdge(mid, to, INF);
					Nt_pp[from * Nt_pp_num_node + (mid)] = 1;
					Nt_pp[mid * Nt_pp_num_node + (to)] = INF;
				}
			}
		}

		std::vector<int> visited_vec;
		visited_vec = g11.FordFulkerson(0, Nt_pp_num_node - 1); // 指定source為vertex(0), termination為vertex(5)

		std::vector<bool> visited_BFS(Nt_pp_num_node, false);
		for (int i : visited_vec)
		{
			// cout << "i = " << i << endl;
			visited_BFS[i] = true;
		}

		if (g11.get_maxflow() > K)
		{
			label[current_node] = p + 1;
			kLUT_list[current_node + 1].push_back(current_node + 1);
		}
		else // <= K
		{
			label[current_node] = p;
			for (int i = 1; i < Nt_pp_num_node - 1; i = i + 2)
			{
				if (visited_BFS[i])
				{
					//cout << i << " ";
					kLUT_list[current_node + 1].push_back(Nt_pp_index[i]);
				}
			}
			for (int i = 0; i < N; i++)
			{
				if (pred_ret[i] == -10 && label[i] == label[current_node])
				{
					kLUT_list[current_node + 1].push_back(i + 1);
				}
			}
		}

		topo_Stack.pop();
	}
}

void topologicalSortUtil(int v, bool visited[], stack<int> &topo_Stack)
{
	// Mark the current node as visited.
	visited[v] = true;

	// Recur for all the vertices adjacent to this vertex
	list<int>::iterator i;
	for (i = adj[v].begin(); i != adj[v].end(); i++)
		if (!visited[*i])
			topologicalSortUtil(*i, visited, topo_Stack);

	// Push current vertex to stack which stores result (except primary input)
	if (!isPI[v])
		topo_Stack.push(v);
}

void topologicalSort()
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
	/*
	// Print contents of stack
	while (topo_Stack.empty() == false)
	{
		cout << topo_Stack.top() << " ";
		topo_Stack.pop();
	}
	*/
}

int *con_network(int Start)
{
	int *predecessor = new int[N]; // -1:沒有predecessor, 表示為起點vertex
	for (int i = 0; i < N; i++)
	{						 // 初始化，如圖二(b)
		predecessor[i] = -1; // -1表示沒有predecessor
	}
	stack<int> temp;
	temp.push(Start);

	vector<int> visited(0, 0);
	visited.push_back(Start);

	predecessor[Start] = -10;
	while (!temp.empty())
	{
		int curr_node = temp.top();
		int count = 0;
		int left_fanin = -1, right_fanin = -1;
		for (std::list<int>::iterator itr = rev_adj[curr_node].begin(); itr != rev_adj[curr_node].end(); itr++)
		{
			switch (count)
			{
			case 0:
				left_fanin = *itr;
				break;
			case 1:
				right_fanin = *itr;
				break;
			default:
				break;
			}
			count++;
		}
		// no fanin
		if (left_fanin == -1 && right_fanin == -1)
		{
			temp.pop();
			visited.push_back(curr_node);
			if (predecessor[curr_node] == -1)
				predecessor[curr_node] = 1;
		}
		else
		{
			if (find(visited.begin(), visited.end(), left_fanin) != visited.end() && find(visited.begin(), visited.end(), right_fanin) != visited.end())
			{
				temp.pop();
				visited.push_back(curr_node);
				if (predecessor[curr_node] == -1)
					predecessor[curr_node] = 1;
			}
			else
			{
				if (find(visited.begin(), visited.end(), left_fanin) == visited.end()) // 尚未visited
					temp.push(left_fanin);
				if (find(visited.begin(), visited.end(), right_fanin) == visited.end())
					temp.push(right_fanin);
			}
		}
	}
	return predecessor;
}

void readFile(const string infile_name)
{
	string line;
	ifstream myfile(infile_name);
	vector<string> tokens;
	if (myfile.is_open())
	{
		string line, name;
		if (getline(myfile, line))
		{
			istringstream ss(line);
			ss >> name >> N >> I >> O;
		}
		// malloc adjacent matrix + input table + output table
		adj = new list<int>[N];
		rev_adj = new list<int>[N];
		kLUT_list = new list<int>[N + 1];
		isPI = new bool[N];
		isPO = new bool[N];
		label = new int[N];

		for (int i = 0; i < N; i++)
		{
			isPI[i] = false;
			isPO[i] = false;
		}
		// set the PI
		for (int i = 0; i < I; i++)
		{
			getline(myfile, line);
			isPI[stoi(line) - 1] = true;
		}
		// set the PO
		for (int i = 0; i < O; i++)
		{
			getline(myfile, line);
			isPO[stoi(line) - 1] = true;
		}
		// Read the ID of the node and its inputs
		for (int i = 0; i < N - I; i++)
		{
			if (getline(myfile, line))
			{
				istringstream ss(line);
				int node_id, input_id;
				ss >> node_id;

				while (ss >> input_id)
				{
					adj[input_id - 1].push_back(node_id - 1);
					rev_adj[node_id - 1].push_back(input_id - 1);
				}
			}
		}
		/*
		for (int i = 0; i < N; i++)
		{
			cout << "Node " << i+1 << ": ";
			for (int neighbor : adj[i])
			{
				cout << neighbor << " ";
			}
			cout << endl;
		}
		*/
		myfile.close();
	}
	else
		cout << "Warning: Failed to open file !";
}

void delete_mem()
{
	delete isPI;
	delete isPO;
}
