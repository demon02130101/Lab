#include "bstar.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <math.h>
#include <random>
#include <cassert>
#include <algorithm>
using namespace std;

random_device rd;
mt19937 gen(rd());
// I/O
void Floorplanner::ReadBlocks(ifstream &input)
{
    // input floorplan info
    string str;
    input >> str;
    assert(str == "Outline:");
    input >> outline_width >> outline_height;
    input >> str;
    assert(str == "NumBlocks:");
    input >> num_blk;
    input >> str;
    assert(str == "NumTerminals:");
    input >> num_tml;
    // input blocks info
    // blk_list.reserve(num_blk); // preallocate memory
    for (int i = 0; i < num_blk; i++)
    {
        string name;
        int width, height;
        input >> name >> width >> height;
        Block *block = new Block(name, width, height);
        blk_list.push_back(block);
        blk_map[name] = blk_list[i];
    }
    // input terminals info
    // tml_list.reserve(num_tml); // preallocate memory
    for (int i = 0; i < num_tml; i++)
    {
        string name, str;
        int x, y;
        input >> name >> str;
        assert(str == "terminal");
        input >> x >> y;
        Terminal *terminal = new Terminal(name, x, y);
        tml_list.push_back(terminal);
        tml_map[name] = tml_list[i];
    }
}
void Floorplanner::ReadNets(ifstream &input)
{
    // input floorplan info
    string str;
    input >> str;
    assert(str == "NumNets:");
    input >> num_net;
    int net_degree;
    // net_list.reserve(num_net); // preallocate memory
    for (int i = 0; i < num_net; i++)
    {
        input >> str;
        assert(str == "NetDegree:");
        input >> net_degree;
        Net *net = new Net();
        net_list.push_back(net);
        // net->cell_list.reserve(net_degree); // preallocate
        for (int j = 0; j < net_degree; j++)
        {
            string name;
            input >> name;
            net_list[i]->cell_list.push_back(name);
        }
    }
}
void Floorplanner::OutputFile(ofstream &output, const chrono::duration<double> &runtime)
{
    Upd_Sol();
    // calculate
    int chip_area = x_max * y_max;
    int total_hpwl = Cal_HPWL();
    int total_cost = alpha * chip_area + (1 - alpha) * total_hpwl;
    //  write the floorplan info
    output << total_cost << endl;
    output << total_hpwl << endl;
    output << chip_area << endl;
    output << x_max << " " << y_max << endl;
    output << runtime.count() << endl;
    // write the block info
    for (int i = 0; i < blk_list.size(); i++)
    {
        Node *node = node_map[blk_list[i]->name];
        output << blk_list[i]->name << " "
               << node->x << " " << node->y << " "
               << node->x + blk_list[i]->w << " " << node->y + blk_list[i]->h << endl;
    }
}
// calculation
double Floorplanner::Cal_Cost()
{
    int x_err = max(x_max - outline_width, 0);
    int y_err = max(y_max - outline_height, 0);
    double area = x_max * y_max;
    double hpwl = Cal_HPWL();
    double norm_area = area / avg_area;
    double norm_hpwl = hpwl / avg_hpwl;

    if (x_err + y_err == 0)
    {
        // double cost = -(outline_width * outline_height) * 1000 / (alpha * area + (1 - alpha) * hpwl);
        double norm_cost = (alpha * norm_area + (1 - alpha) * norm_hpwl);
        // cout << "OK = " << norm_cost << endl;
        return norm_cost;
    }
    else // x_max or y_max is out of range
    {
        // cout << "Err = " << (x_err + y_err) * 100 << endl;
        return (x_err + y_err) * 100;
    }
}
double Floorplanner::Cost()
{
    int x_err = max(x_max - outline_width, 0);
    int y_err = max(y_max - outline_height, 0);
    double area = x_max * y_max;
    double hpwl = Cal_HPWL();
    double norm_area = area / avg_area;
    double norm_hpwl = hpwl / avg_hpwl;

    double norm_cost = (alpha * norm_area + (1 - alpha) * norm_hpwl);
    return norm_cost;
}
int Floorplanner::Cal_HPWL()
{
    int total_HPWL = 0;
    for (int i = 0; i < net_list.size(); i++)
    {
        int minX = INT32_MAX, maxX = 0;
        int minY = INT32_MAX, maxY = 0;
        for (int j = 0; j < net_list[i]->cell_list.size(); j++)
        {
            string name = net_list[i]->cell_list[j];
            int posX, posY;
            if (blk_map.count(name))
            {
                Block *curr_blk = blk_map[name];
                Node *curr_node = node_map[name];
                posX = curr_node->x + curr_blk->w / 2; // centerX of blk
                posY = curr_node->y + curr_blk->h / 2; // centerY of blk
            }
            else if (tml_map.count(name))
            {
                Terminal *curr_tml = tml_map[name];
                posX = curr_tml->x;
                posY = curr_tml->y;
            }
            else
            {
                cout << "no matching cell" << endl;
            }
            minX = min(posX, minX);
            maxX = max(posX, maxX);
            minY = min(posY, minY);
            maxY = max(posY, maxY);
        }
        total_HPWL += (maxX - minX) + (maxY - minY);
    }
    return total_HPWL;
}
void Floorplanner::Cal_Avg()
{
    BuildTree();
    double area, hpwl;
    double total_area, total_hpwl;
    for (int i = 0; i < 2 * num_blk; i++)
    {
        uniform_int_distribution<> dis(0, 2);
        int op = dis(gen);
        // Rotate
        if (op == 0)
        {
            uniform_int_distribution<> dis(0, num_blk - 1);
            int id = dis(gen);
            swap(blk_list[id]->w, blk_list[id]->h);

            Upd_Sol();
            area = x_max * y_max;
            hpwl = Cal_HPWL();
        }
        // Move
        if (op == 1)
        {
            int del_node;
            int ins_node;
            Node *delNode, *insNode;
            while (true)
            {
                uniform_int_distribution<> dis(1, num_blk - 1); // from 1 because not choosing root
                uniform_int_distribution<> dis2(0, num_blk - 1);
                del_node = dis(gen);
                ins_node = dis2(gen);
                delNode = node_list[del_node];
                insNode = node_list[ins_node];
                if ((delNode != insNode) &&
                    (insNode->left == NULL || insNode->right == NULL) &&
                    (delNode->left == NULL || delNode->right == NULL))
                {
                    break;
                }
            }

            Move_Node(del_node, ins_node);

            Upd_Sol();
            area = x_max * y_max;
            hpwl = Cal_HPWL();
        }
        // Swap
        if (op == 2)
        {
            uniform_int_distribution<> dis(0, num_blk - 1);
            int from_node = dis(gen);
            int to_node = dis(gen);
            while (from_node == to_node)
            {
                to_node = dis(gen);
            }
            Swap_Node(from_node, to_node);

            Upd_Sol();
            area = x_max * y_max;
            hpwl = Cal_HPWL();
        }
        total_area += area;
        total_hpwl += hpwl;
        avg_area = total_area / (2 * num_blk);
        avg_hpwl = total_hpwl / (2 * num_blk);
    }
    DelTree();
}
void Floorplanner::Cal_deltaAvg()
{
    BuildTree();
    int bad_sol = 0;
    double total_delta_cost;
    double new_cost, old_cost, delta_cost;
    int area, hpwl;

    total_delta_cost = new_cost = old_cost = delta_cost = 0;
    for (int i = 0; i < 2 * num_blk; i++)
    {
        Upd_Sol();
        old_cost = Cost();

        uniform_int_distribution<> dis(0, 2);
        int op = dis(gen);
        // Rotate
        if (op == 0)
        {
            uniform_int_distribution<> dis(0, num_blk - 1);
            int id = dis(gen);
            swap(blk_list[id]->w, blk_list[id]->h);

            Upd_Sol();
            new_cost = Cost();

            delta_cost = new_cost - old_cost;
            if (delta_cost > 0)
            {
                bad_sol++;
                total_delta_cost += delta_cost;
                swap(blk_list[id]->w, blk_list[id]->h);
            }
        }
        // Move
        if (op == 1)
        {
            int del_node;
            int ins_node;
            Node *delNode, *insNode;
            while (true)
            {
                uniform_int_distribution<> dis(1, num_blk - 1); // from 1 because not choosing root
                uniform_int_distribution<> dis2(0, num_blk - 1);
                del_node = dis(gen);
                ins_node = dis2(gen);
                delNode = node_list[del_node];
                insNode = node_list[ins_node];
                if ((delNode != insNode) &&
                    (insNode->left == NULL || insNode->right == NULL) &&
                    (delNode->left == NULL || delNode->right == NULL))
                {
                    break;
                }
            }

            int is_LR; // 0 for left, 1 for right
            Node *parentNode = delNode->parent;
            if (delNode->parent->left == delNode)
                is_LR = 0;
            else
                is_LR = 1;

            int child_LR; // 0 for left, 1 for right, 2 for NULL
            Node *childNode;
            if (delNode->left != NULL)
            {
                childNode = delNode->left;
                child_LR = 0;
            }
            else if (delNode->right != NULL)
            {
                childNode = delNode->right;
                child_LR = 1;
            }
            else
                child_LR = 2;

            Move_Node(del_node, ins_node);

            Upd_Sol();
            new_cost = Cost();

            delta_cost = new_cost - old_cost;
            if (delta_cost > 0)
            {
                bad_sol++;
                total_delta_cost += delta_cost;
                // set insnode
                if (insNode->left == delNode)
                    insNode->left = NULL;
                else
                    insNode->right = NULL;

                if (child_LR == 0)
                {
                    delNode->left = childNode;
                    childNode->parent = delNode;
                }
                else if (child_LR == 1)
                {
                    delNode->right = childNode;
                    childNode->parent = delNode;
                }

                // set parent and delnode
                if (is_LR == 0)
                {
                    parentNode->left = delNode;
                    delNode->parent = parentNode;
                }
                else if (is_LR == 1)
                {
                    parentNode->right = delNode;
                    delNode->parent = parentNode;
                }
            }
        }
        // Swap
        if (op == 2)
        {
            uniform_int_distribution<> dis(0, num_blk - 1);
            int from_node = dis(gen);
            int to_node = dis(gen);
            while (from_node == to_node)
            {
                to_node = dis(gen);
            }
            Swap_Node(from_node, to_node);

            Upd_Sol();
            new_cost = Cost();

            delta_cost = new_cost - old_cost;
            if (delta_cost > 0)
            {
                bad_sol++;
                total_delta_cost += delta_cost;
                Swap_Node(from_node, to_node);
            }
        }
    }
    deltaAvg = total_delta_cost / bad_sol;
    DelTree();
    // cout << "no. of MT = " << 2 * num_blk << endl;
    // cout << "bad sol = " << bad_sol << endl;
    // cout << "toal delta_cost = " << total_delta_cost << endl;
    // cout << "deltaAvg = " << deltaAvg << endl;
}

// Perturbation
void Floorplanner::Swap_Node(int node1, int node2)
{
    // Get the blocks and nodes to be swapped
    Block *blk1 = blk_map[node_list[node1]->name];
    Block *blk2 = blk_map[node_list[node2]->name];
    // Swap blocks in the blk_map
    swap(blk_map[blk1->name], blk_map[blk2->name]);
    // Swap nodes in the node_map
    swap(node_map[blk1->name], node_map[blk2->name]);
    // Swap block properties (name, width, height)
    swap(blk1->name, blk2->name);
    swap(blk1->w, blk2->w);
    swap(blk1->h, blk2->h);
    // Swap the names of the nodes in the node_list
    swap(node_list[node1]->name, node_list[node2]->name);
}
void Floorplanner::Move_Node(int from_node, int to_node)
{
    // Deletion part
    Node *delNode = node_list[from_node];
    Node *parentNode = delNode->parent;

    if (delNode->left != nullptr || delNode->right != nullptr)
    {
        Node *childNode = (delNode->left != nullptr) ? delNode->left : delNode->right;

        childNode->parent = parentNode;
        if (parentNode->left == delNode)
            parentNode->left = childNode;
        else
            parentNode->right = childNode;
    }
    else // no children
    {
        if (parentNode->left == delNode)
            parentNode->left = nullptr;
        else
            parentNode->right = nullptr;
    }

    // Clear left and right pointers of the deleted node
    delNode->left = delNode->right = nullptr;

    // Insertion part
    Node *insNode = node_list[to_node];
    if (insNode->left == NULL && insNode->right == NULL)
    {
        (rand() & 1) ? insNode->left = delNode : insNode->right = delNode;
    }
    else if (insNode->left == NULL)
        insNode->left = delNode;
    else if (insNode->right == NULL)
        insNode->right = delNode;
    else // invalid case
        cout << "destination node is FULL!" << endl;
    // Update the parent pointer
    delNode->parent = insNode;
}
// floorplanning
void Floorplanner::BuildTree()
{
    for (int i = 0; i < num_blk; i++)
    {
        string name = blk_list[i]->name;
        node_list.push_back(new Node(name));
        if (node_map.count(name) == 0)
            node_map[name] = node_list[i];
    }
    // build a balance tree
    root = node_list[0];
    queue<Node *> q;
    q.push(root);
    for (int i = 1; i < num_blk; i++)
    {
        Node *current_node = node_list[i];
        Node *parent_node = q.front();

        if (parent_node->left == nullptr)
        {
            parent_node->left = current_node;
        }
        else if (parent_node->right == nullptr)
        {
            parent_node->right = current_node;
            q.pop();
        }
        current_node->parent = parent_node;
        q.push(current_node);
    }
    SetPos(root); // set SetPos and calculate x_max, y_max
}
void Floorplanner::DelTree(Node *node)
{
    if (node == nullptr)
    {
        return;
    }
    DelTree(node->left);
    DelTree(node->right);
    delete node;
}
void Floorplanner::DelTree()
{
    DelTree(root);

    node_list.clear();
    node_map.clear();

    root = nullptr;
}
void Floorplanner::SetPos(Node *node)
{
    if (node != NULL)
    {
        if (node->parent == NULL)
        {
            node->x = 0;
            node->y = 0;
            contour_list.push_back(Contour(0, blk_map[node->name]->w, blk_map[node->name]->h));
            y_max = blk_map[node->name]->h;
            x_max = blk_map[node->name]->w;
        }
        else // node == node->parent->left or right
        {
            int x = node->parent->x + (node == node->parent->left ? blk_map[node->parent->name]->w : 0);
            node->x = x;
            x_max = max(x + blk_map[node->name]->w, x_max);
            node->y = SetContour_Caly(node);
        }
        SetPos(node->left);
        SetPos(node->right);
    }
}
int Floorplanner::SetContour_Caly(Node *node)
{
    int y2, y1 = 0;
    int x1 = node->x;
    int x2 = x1 + blk_map[node->name]->w;
    vector<Contour>::iterator it = contour_list.begin();
    while (it != contour_list.end())
    {
        if (it->x2 <= x1)
        {
            it++;
        }
        else if (it->x1 >= x2)
        {
            break;
        }
        else
        {
            y1 = max(it->y, y1);

            if (it->x1 >= x1 && it->x2 <= x2)
            {
                it = contour_list.erase(it);
            }
            else if (it->x1 >= x1 && it->x2 >= x2)
            {
                it->x1 = x2;
            }
            else if (it->x1 <= x1 && it->x2 <= x2)
            {
                it->x2 = x1;
            }
            else // it->x1 <= x1 && it->x2 >= x2
            {
                contour_list.insert(it, Contour(it->x1, x1, it->y));
                it->x1 = x2;
            }
        }
    }
    y2 = y1 + blk_map[node->name]->h;
    contour_list.insert(it, Contour(x1, x2, y2));
    y_max = max(y_max, y2);

    return y1;
}
void Floorplanner::Upd_Sol()
{
    x_max = 0;
    y_max = 0;
    contour_list.clear();
    SetPos(root);
}
void Floorplanner::SA()
{
    auto start_time = chrono::high_resolution_clock::now();
    auto current_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed;
    // cout << "Test 1" << endl;
    Cal_Avg();
    Cal_deltaAvg();
    BuildTree();
    double T = deltaAvg / abs(log(0.9));
    // cout << "T1 = " << T << endl;
    //  double T = 1000;
    int reject = 0;
    int MT = 0;
    double reject_rate;
    best_cost = (double)INT32_MAX;
    double new_cost, old_cost, delta_cost;
    uniform_real_distribution<> disexp(0.0, 1.0);
    while (true)
    {
        reject = 0;
        for (int i = 0; i < 2 * num_blk; i++)
        {
            current_time = std::chrono::high_resolution_clock::now();
            elapsed = current_time - start_time;
            if (elapsed.count() > 280.0)
            {
                // cout << "time out!" << endl;
                // cout << "best cost = " << alpha * best_area + (1 - alpha) * best_hpwl << endl;
                // cout << "MT = " << MT << endl;
                // cout << "T = " << T << endl;
                // cout << "reject rate = " << reject_rate << endl;
                return;
            }

            Upd_Sol();
            old_cost = Cal_Cost();

            uniform_int_distribution<> dis(0, 2);
            int op = dis(gen);
            // Rotate
            if (op == 0)
            {
                uniform_int_distribution<> dis(0, num_blk - 1);
                int id = dis(gen);
                swap(blk_list[id]->w, blk_list[id]->h);

                Upd_Sol();
                new_cost = Cal_Cost();

                delta_cost = new_cost - old_cost;
                double random = disexp(gen);
                if (delta_cost > 0 && random >= exp(-delta_cost / T))
                {
                    reject++;
                    swap(blk_list[id]->w, blk_list[id]->h);
                }
            }
            // Move
            if (op == 1)
            {
                int del_node;
                int ins_node;
                Node *delNode, *insNode;
                while (true)
                {
                    uniform_int_distribution<> dis(1, num_blk - 1); // from 1 because not choosing root
                    uniform_int_distribution<> dis2(0, num_blk - 1);
                    del_node = dis(gen);
                    ins_node = dis2(gen);
                    delNode = node_list[del_node];
                    insNode = node_list[ins_node];
                    if ((delNode != insNode) &&
                        (insNode->left == NULL || insNode->right == NULL) &&
                        (delNode->left == NULL || delNode->right == NULL))
                    {
                        break;
                    }
                }

                int is_LR; // 0 for left, 1 for right
                Node *parentNode = delNode->parent;
                if (delNode->parent->left == delNode)
                    is_LR = 0;
                else
                    is_LR = 1;

                int child_LR; // 0 for left, 1 for right, 2 for NULL
                Node *childNode;
                if (delNode->left != NULL)
                {
                    childNode = delNode->left;
                    child_LR = 0;
                }
                else if (delNode->right != NULL)
                {
                    childNode = delNode->right;
                    child_LR = 1;
                }
                else
                    child_LR = 2;

                Move_Node(del_node, ins_node);

                Upd_Sol();
                new_cost = Cal_Cost();

                delta_cost = new_cost - old_cost;
                double random = disexp(gen);
                if (delta_cost > 0 && random >= exp(-delta_cost / T))
                {
                    reject++;
                    // set insnode
                    if (insNode->left == delNode)
                        insNode->left = NULL;
                    else
                        insNode->right = NULL;

                    if (child_LR == 0)
                    {
                        delNode->left = childNode;
                        childNode->parent = delNode;
                    }
                    else if (child_LR == 1)
                    {
                        delNode->right = childNode;
                        childNode->parent = delNode;
                    }

                    // set parent and delnode
                    if (is_LR == 0)
                    {
                        parentNode->left = delNode;
                        delNode->parent = parentNode;
                    }
                    else if (is_LR == 1)
                    {
                        parentNode->right = delNode;
                        delNode->parent = parentNode;
                    }
                }
            }
            // Swap
            if (op == 2)
            {
                uniform_int_distribution<> dis(0, num_blk - 1);
                int from_node = dis(gen);
                int to_node = dis(gen);
                while (from_node == to_node)
                {
                    to_node = dis(gen);
                }
                Swap_Node(from_node, to_node);

                Upd_Sol();
                new_cost = Cal_Cost();

                delta_cost = new_cost - old_cost;
                double random = disexp(gen);
                if (delta_cost > 0 && random >= exp(-delta_cost / T))
                {
                    reject++;
                    Swap_Node(from_node, to_node);
                }
            }
            // update best cost
            if (delta_cost <= 0 && new_cost < best_cost)
            {
                best_cost = new_cost;
                best_xmax = x_max;
                best_ymax = y_max;
                best_area = best_xmax * best_ymax;
                best_hpwl = Cal_HPWL();
            }
            MT++;
        }

        T *= 0.8;
        reject_rate = (double)reject / (2 * num_blk);
        if ((reject_rate > 0.95) && (x_max - outline_width <= 0) && (y_max - outline_height <= 0) && elapsed.count() > 250)
        {
            // cout << "reject rate too high!" << endl;
            // cout << "best cost = " << alpha * best_area + (1 - alpha) * best_hpwl << endl;
            // cout << "MT = " << MT << endl;
            // cout << "T = " << T << endl;
            // cout << "reject rate = " << reject_rate << endl;
            return;
        }
        if (T < 0.00001 && (x_max - outline_width <= 0) && (y_max - outline_height <= 0) && elapsed.count() > 250)
        {
            // cout << "T is small enough!" << endl;
            // cout << "best cost = " << alpha * best_area + (1 - alpha) * best_hpwl << endl;
            // cout << "MT = " << MT << endl;
            // cout << "T = " << T << endl;
            // cout << "reject rate = " << reject_rate << endl;
            return;
        }
    }
}
// Debug
void Floorplanner::All_blkArea()
{
    int total_area = 0;
    for (const auto *block : blk_list)
    {
        if (block != nullptr)
        {
            total_area += block->area();
        }
    }
    cout << "total block area = " << total_area << endl;
}
void Floorplanner::Cal_Tsteps(double T, double end_T, double rate)
{
    if (rate <= 0 || rate >= 1 || T <= end_T)
    {
        std::cerr << "Invalid input values." << std::endl;
    }
    cout << "MAX Steps = " << ceil(log(end_T / T) / log(rate)) << endl;
}