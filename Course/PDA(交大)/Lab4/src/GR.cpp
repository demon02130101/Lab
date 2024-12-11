#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <limits>
#include <unordered_set>
#include <cmath>
#include "GR.h"

using namespace std;

GR::GR(string gmp_file, string gcl_file, string cst_file, string output_file)
{
    read_gmp(gmp_file);
    Gcells.resize(num_row, vector<Gcell>(num_col));
    read_gcl(gcl_file);
    read_cst(cst_file);
    Run();
    write_ans(output_file);
}

// I/O
void GR::read_gmp(string gmp_file)
{
    ifstream input(gmp_file);
    if (!input)
    {
        cerr << "Error: can not open file " << gmp_file << endl;
        exit(1);
    }
    string str, line;

    input >> str; // .ra
    input >> area_x >> area_y >> area_w >> area_h;

    input >> str; // .g
    input >> grid_w >> grid_h;

    input >> str; // .c1
    input >> chip1_x >> chip1_y >> chip1_w >> chip1_h;

    input >> str; // .b
    input.ignore();
    while (getline(input, line))
    {
        if (line.empty())
            break;
        stringstream ss(line);
        Bump bump;
        ss >> bump.id >> bump.x >> bump.y;
        // area_pos
        bump.x += chip1_x;
        bump.y += chip1_y;
        s_bumps.push_back(bump);
    }

    input >> str; // .c2
    input >> chip2_x >> chip2_y >> chip2_w >> chip2_h;

    input >> str; // .b
    input.ignore();
    while (getline(input, line))
    {
        stringstream ss(line);
        Bump bump;
        ss >> bump.id >> bump.x >> bump.y;
        // area_pos
        bump.x += chip2_x;
        bump.y += chip2_y;
        t_bumps.push_back(bump);
    }

    num_row = area_h / grid_h;
    num_col = area_w / grid_w;
    // for (auto it = t_bumps.begin(); it != t_bumps.end(); it++)
    // {
    //     cout << it->id << " " << it->x << " " << it->y << endl;
    // }
}

void GR::read_gcl(string gcl_file)
{
    ifstream input(gcl_file);
    if (!input)
    {
        cerr << "Error: can not open file " << gcl_file << endl;
        exit(1);
    }
    string str, line;

    input >> str; // .ec
    for (int i = 0; i < num_row; i++)
    {
        for (int j = 0; j < num_col; j++)
        {
            Gcell gcl;
            input >> gcl.left_cap >> gcl.bottom_cap;
            gcl.y = i * grid_h;
            gcl.x = j * grid_w;
            gcl.id_y = i;
            gcl.id_x = j;
            Gcells[i][j] = gcl;
        }
    }

    // for (int i = 0; i < Gcells.size(); ++i)
    // {
    //     for (auto it = Gcells[i].begin(); it != Gcells[i].end(); it++)
    //     {
    //         cout << it->left_cap << " " << it->bottom_cap << " ";
    //     }
    //     cout << endl;
    // }

    // for (int i = 0; i < Gcells.size(); ++i)
    // {
    //     for (int j = 0; j < Gcells[i].size(); j++)
    //     {
    //         auto it = &Gcells[i][j];
    //         cout << "Gcell [" << i << "][" << j << "] :"
    //              << it->x << " " << it->y << endl;
    //     }
    //     cout << endl;
    // }
}

void GR::read_cst(string cst_file)
{
    ifstream input(cst_file);
    if (!input)
    {
        cerr << "Error: can not open file " << cst_file << endl;
        exit(1);
    }
    string str, line;

    input >> str;
    assert(str == ".alpha");
    input >> alpha;

    input >> str;
    assert(str == ".beta");
    input >> beta;

    input >> str;
    assert(str == ".gamma");
    input >> gamma;

    input >> str;
    assert(str == ".delta");
    input >> delta;

    input >> str; // .v
    input >> via_cost;

    input >> str; // .l
    // input.ignore();
    double gcl_cost;
    for (int i = 0; i < num_row; i++)
    {
        for (int j = 0; j < num_col; j++)
        {
            input >> gcl_cost;
            Gcells[i][j].M1_cost = gcl_cost;
        }
    }

    input >> str; //.l

    for (int i = 0; i < num_row; i++)
    {
        for (int j = 0; j < num_col; j++)
        {
            input >> gcl_cost;
            Gcells[i][j].M2_cost = gcl_cost;
        }
    }

    // for (int i = 0; i < Gcells.size(); ++i)
    // {
    //     for (int j = 0; j < Gcells[i].size(); j++)
    //     {
    //         auto it = &Gcells[i][j];
    //         cout << "Gcell [" << i << "][" << j << "]"
    //              << it->left_cap << " " << it->bottom_cap << " "
    //              << it->M1_cost << " " << it->M2_cost << endl;
    //     }
    //     cout << endl;
    // }
}

void GR::write_ans(string output_file)

{
    ofstream output(output_file);
    for (int i = 0; i < s_bumps.size(); i++)
    {
        output << "n" << i + 1 << endl;
        int start_x, start_y, end_x, end_y;
        // first two nodes
        Node *fir = &s_bumps[i].net[0];
        Node *sec = &s_bumps[i].net[1];
        start_x = fir->x + area_x;
        start_y = fir->y + area_y;
        if (fir->dir == sec->dir)
        {
            output << "M1 ";
        }
        else
        {
            output << "via" << endl;
            output << "M2 ";
        }
        // intermediate nodes
        for (int j = 1; j < s_bumps[i].net.size() - 1; j++)
        {
            Node *cur = &s_bumps[i].net[j];
            Node *next = &s_bumps[i].net[j + 1];
            if (cur->dir == next->dir)
            {
                continue;
            }
            else if (cur->dir != next->dir)
            {
                end_x = cur->x + area_x;
                end_y = cur->y + area_y;
                output << start_x << " " << start_y << " " << end_x << " " << end_y << endl;

                output << "via" << endl;
                if (next->dir == M1)
                {
                    output << "M1 ";
                }
                else if (next->dir == M2)
                {
                    output << "M2 ";
                }
                start_x = cur->x + area_x;
                start_y = cur->y + area_y;
            }
        }
        // last node
        Node *r_fir = &s_bumps[i].net.back();
        end_x = r_fir->x + area_x;
        end_y = r_fir->y + area_y;
        output << start_x << " " << start_y << " " << end_x << " " << end_y << endl;
        if (r_fir->dir == M2)
            output << "via" << endl;

        output << ".end" << endl;
    }
}

// route
void GR::Run()
{
    for (int i = 0; i < s_bumps.size(); i++)
        A_star(s_bumps[i], t_bumps[i]);
}

void GR::A_star(Bump &s_bump, Bump &t_bump)
{
    vector<vector<Node>> nodes;
    init_nodes(Gcells, nodes);
    Node *s_node, *t_node;

    int s_x, s_y, t_x, t_y;
    s_y = s_bump.y / grid_h;
    s_x = s_bump.x / grid_w;
    t_y = t_bump.y / grid_h;
    t_x = t_bump.x / grid_w;

    s_node = &nodes[s_y][s_x];
    t_node = &nodes[t_y][t_x];

    // Step 1 mark source as open and cal f(s)
    vector<Node *> closed_set;
    vector<Node *> open_set;

    s_node->dir = M1;
    s_node->g_cost = 0;
    s_node->h_cost = 0 + cal_g(t_node, s_node);
    s_node->f_cost = s_node->g_cost + s_node->h_cost;
    open_set.push_back(s_node);

    // Step 2 select an open node n from open_set with smallest f(n)
    Node *cur_node;
    while (!open_set.empty())
    {
        cur_node = select_node(open_set, t_node);
        if (cur_node->x == t_bump.x && cur_node->y == t_bump.y) // check cur_node == target
        {
            open_set.erase(remove(open_set.begin(), open_set.end(), cur_node), open_set.end());
            closed_set.push_back(cur_node);
            construct_net(s_bump, cur_node);
            return;
        }
        else
        {
            // open_set.erase(&cur_node);
            open_set.erase(remove(open_set.begin(), open_set.end(), cur_node), open_set.end());
            closed_set.push_back(cur_node);
            // apply successor
            Node *next_node;
            vector<Node *> neighbors;

            if (cur_node->id_y != num_row - 1) // check if valid
            {
                next_node = &nodes[cur_node->id_y + 1][cur_node->id_x]; // top nbr
                next_node->temp_dir = M1;
                neighbors.push_back(next_node);
            }
            if (cur_node->id_y != 0)
            {
                next_node = &nodes[cur_node->id_y - 1][cur_node->id_x]; // bottom nbr
                next_node->temp_dir = M1;
                neighbors.push_back(next_node);
            }
            if (cur_node->id_x != 0)
            {
                next_node = &nodes[cur_node->id_y][cur_node->id_x - 1]; // left nbr
                next_node->temp_dir = M2;
                neighbors.push_back(next_node);
            }
            if (cur_node->id_x != num_col - 1)
            {
                next_node = &nodes[cur_node->id_y][cur_node->id_x + 1]; // right nbr
                next_node->temp_dir = M2;
                neighbors.push_back(next_node);
            }

            for (auto &nbr_node : neighbors)
            {
                // 如果在 closed_set 中，跳過該節點
                if (find(closed_set.begin(), closed_set.end(), nbr_node) != closed_set.end())
                {
                    continue;
                }

                nbr_node->dir = nbr_node->temp_dir;
                // cal g_cost
                double temp_g_cost = cur_node->g_cost + cal_g(cur_node, nbr_node);
                bool better;
                // 如果不在 open_set 中，加入 open_set
                if (find(open_set.begin(), open_set.end(), nbr_node) == open_set.end())
                {
                    better = true;
                }
                else if (temp_g_cost < nbr_node->g_cost)
                {
                    better = true;
                }
                else
                {
                    better = false;
                }

                if (better == true)
                {
                    nbr_node->parent = cur_node;
                    // cout << nbr_node->id_y << " " << nbr_node->id_x << " parent is " << nbr_node->parent->id_y << " " << nbr_node->parent->id_x << endl;
                    nbr_node->g_cost = temp_g_cost;
                    // cout << nbr_node->g_cost << endl;
                    nbr_node->h_cost = cal_h(t_node, nbr_node);
                    // cout << nbr_node->h_cost << endl;
                    nbr_node->f_cost = nbr_node->g_cost + nbr_node->h_cost;
                    // cout << nbr_node->f_cost << endl;
                    if (find(open_set.begin(), open_set.end(), nbr_node) == open_set.end())
                    {
                        open_set.push_back(nbr_node);
                        // cout << open_set.back().id_y << " " << open_set.back().id_x << " parent is " << open_set.back().parent->id_y << " " << open_set.back().parent->id_x << endl;
                    }
                }
            }
        }
    }
}

void GR::init_nodes(const vector<vector<Gcell>> &Gcells, vector<vector<Node>> &nodes)
{
    nodes.resize(Gcells.size());
    for (size_t i = 0; i < Gcells.size(); ++i)
    {
        nodes[i].resize(Gcells[i].size());
        for (size_t j = 0; j < Gcells[i].size(); ++j)
        {
            // 將 Gcell 的參數複製到 Node
            nodes[i][j].x = Gcells[i][j].x;
            nodes[i][j].y = Gcells[i][j].y;
            nodes[i][j].id_x = Gcells[i][j].id_x;
            nodes[i][j].id_y = Gcells[i][j].id_y;
            nodes[i][j].left_cap = Gcells[i][j].left_cap;
            nodes[i][j].bottom_cap = Gcells[i][j].bottom_cap;
            nodes[i][j].num_left = Gcells[i][j].num_left;
            nodes[i][j].num_bottom = Gcells[i][j].num_bottom;
            nodes[i][j].M1_cost = Gcells[i][j].M1_cost;
            nodes[i][j].M2_cost = Gcells[i][j].M2_cost;
        }
    }
}

Node *GR::select_node(vector<Node *> &open_set, Node *t_node)
{
    // 設置初始最小值為最大 double 值
    double min_f_cost = numeric_limits<double>::max();
    Node *sel_node = nullptr;
    vector<Node *> min_set;

    // 遍歷 open_set 中的指針以找到 f_cost 最小的節點
    for (Node *node : open_set)
    {
        if (node->f_cost < min_f_cost)
        {
            min_f_cost = node->f_cost;
        }
    }

    for (Node *node : open_set)
    {
        if (node->f_cost == min_f_cost)
        {
            min_set.push_back(node);
        }
    }

    double cst;
    double min_cost = numeric_limits<double>::max();
    for (Node *node : min_set)
    {
        // dis= abs(t_node->x - node->x) + abs(t_node->y - node->y);
        cst = node->g_cost;
        if (cst < min_cost)
        {
            min_cost = cst;
            sel_node = node;
        }
    }

    return sel_node;
}

void GR::construct_net(Bump &s_bump, Node *t_node)
{
    Node *current = t_node;
    // 回溯路徑直到起點
    while (current != nullptr)
    {
        s_bump.net.push_back(*current); // 存儲節點的copy
        current = current->parent;      // 回溯到父節點
    }

    // 反轉路徑，使其從起點到終點的順序
    reverse(s_bump.net.begin(), s_bump.net.end());

    Node *cur, *next;
    for (int i = 0; i < s_bump.net.size() - 1; i++)
    {
        cur = &s_bump.net[i];
        next = &s_bump.net[i + 1];
        if (next->id_y == cur->id_y + 1) // 下一個在上面
        {
            Gcells[next->id_y][next->id_x].num_bottom++;
            next->dir = M1;
        }
        else if (next->id_y == cur->id_y - 1) // // 下一個在下面
        {
            Gcells[cur->id_y][cur->id_x].num_bottom++;
            next->dir = M1;
        }
        else if (next->id_x == cur->id_x - 1) // 下一個在左
        {
            Gcells[cur->id_y][cur->id_x].num_left++;
            next->dir = M2;
        }
        else if (next->id_x == cur->id_x + 1) // 下一個在右
        {
            Gcells[next->id_y][next->id_x].num_left++;
            next->dir = M2;
        }
    }

    // for (auto &node : s_bump.net)
    // {
    //     cout << "(" << node.id_y << ", " << node.id_x << ") " << node.dir << " ";
    // }
    // cout << endl;
    // for (auto &node : s_bump.net)
    // {
    //     cout << node.dir << " ";
    // }
    // cout << endl;
}

// cal
double GR::cal_g(Node *cur_node, Node *next_node)
{
    double g_cst;
    double WL, OV, cell_cst, via_cst;
    WL = abs(next_node->x - cur_node->x) + abs(next_node->y - cur_node->y);

    // cell cost and via cost
    if (cur_node->dir == M1)
    {
        if (cur_node->dir == next_node->dir)
        {
            cell_cst = cur_node->M1_cost;
            via_cst = 0;
        }
        else
        {
            cell_cst = (cur_node->M1_cost + cur_node->M2_cost) / 2;
            via_cst = this->via_cost;
        }
    }
    else if (cur_node->dir == M2)
    {
        if (cur_node->dir == next_node->dir)
        {
            cell_cst = cur_node->M2_cost;
            via_cst = 0;
        }
        else
        {
            cell_cst = (cur_node->M1_cost + cur_node->M2_cost) / 2;
            via_cst = this->via_cost;
        }
    }

    // overflow cost
    if (next_node->id_x == cur_node->id_x + 1)
    {
        OV = Gcells[next_node->id_y][next_node->id_x].num_left - next_node->left_cap;
    }
    else if (next_node->id_y == cur_node->id_y + 1)
    {
        OV = Gcells[next_node->id_y][next_node->id_x].num_bottom - next_node->bottom_cap;
    }
    else if (next_node->id_x == cur_node->id_x - 1)
    {
        OV = Gcells[cur_node->id_y][cur_node->id_x].num_left - cur_node->left_cap;
    }
    else if (next_node->id_y == cur_node->id_y - 1)
    {
        OV = Gcells[cur_node->id_y][cur_node->id_x].num_bottom - cur_node->bottom_cap;
    }
    else
        OV = 0;
    if (OV < 0)
        OV = 0;

    g_cst = pow(alpha, 5) * WL + pow(beta, 5) * OV + pow(gamma, 5) * cell_cst + pow(delta, 5) * via_cst;
    return g_cst;
}

double GR::cal_h(Node *t_node, Node *next_node)
{
    double h_cst;
    double WL, total_OV, cell_cst, via_cst;
    WL = abs(t_node->x - next_node->x) + abs(t_node->y - next_node->y);

    // cell cost and OV cost
    double M1_csts, M2_csts;
    double OV;
    int dis_x, dis_y;
    dis_x = abs(t_node->id_x - next_node->id_x);
    dis_y = abs(t_node->id_y - next_node->id_y);

    M1_csts = M2_csts = total_OV = 0;
    for (int i = 0; i < dis_x; i++)
    {
        if (t_node->id_x > next_node->id_x)
        {
            M2_csts += Gcells[next_node->id_y][next_node->id_x + i].M2_cost;
            OV = Gcells[next_node->id_y][next_node->id_x + i].num_left - Gcells[next_node->id_y][next_node->id_x + i].left_cap;
            if (OV > 0)
            {
                total_OV += OV;
            }
        }
        else
        {
            M2_csts += Gcells[next_node->id_y][next_node->id_x - i].M2_cost;
            OV = Gcells[next_node->id_y][next_node->id_x - i].num_left - Gcells[next_node->id_y][next_node->id_x - i].left_cap;
            if (OV > 0)
            {
                total_OV += OV;
            }
        }
    }

    for (int i = 0; i < dis_y; i++)
    {
        if (t_node->id_y > next_node->id_y)
        {
            M1_csts += Gcells[next_node->id_y + i][next_node->id_x].M1_cost;
            OV = Gcells[next_node->id_y + i][next_node->id_x].num_bottom - Gcells[next_node->id_y + i][next_node->id_x].bottom_cap;
            if (OV > 0)
            {
                total_OV += OV;
            }
        }

        else
        {
            M1_csts += Gcells[next_node->id_y - i][next_node->id_x].M1_cost;
            OV = Gcells[next_node->id_y - i][next_node->id_x].num_bottom - Gcells[next_node->id_y - i][next_node->id_x].bottom_cap;
            if (OV > 0)
            {
                total_OV += OV;
            }
        }
    }
    cell_cst = M1_csts + M2_csts;

    // via cost
    if (next_node->dir == M1)
    {
        if (next_node->x == t_node->x)
        {
            via_cst = 0;
        }
        else
        {
            via_cst = this->via_cost;
        }
    }
    else if (next_node->dir == M2)
    {
        if (next_node->y == t_node->y)
        {
            via_cst = 0;
        }
        else
        {
            via_cst = this->via_cost;
        }
    }

    h_cst = pow(alpha, 5) * WL + pow(beta, 5) * total_OV + pow(gamma, 5) * cell_cst + pow(delta, 5) * via_cst;
    return h_cst;
}