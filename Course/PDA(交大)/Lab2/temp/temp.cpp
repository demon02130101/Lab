
void Floorplanner::BuildTree()
{
    cout << "Test 1" << endl;
    for (int i = 0; i < num_blk; ++i)
    {
        string name = blk_list[i]->name;
        node_list.push_back(new Node(name));
        if (node_map.count(name) == 0)
            node_map[name] = node_list[i];
    }
    root = node_list[0];
    root->left = node_list[1];
    node_list[1]->parent = root;

    for (int i = 1; i < num_blk - 1; i++)
    {
        node_list[i]->left = node_list[i + 1];
        node_list[i + 1]->parent = node_list[i];
    }
    SetPos(root); // set SetPos and calculate x_max, y_max
}
void Floorplanner::BuildTree()
{
    cout << "Test 1" << endl;
    for (int i = 0; i < num_blk; ++i)
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
    for (int i = 1; i < num_blk; ++i)
    {
        Node *current_node = node_list[i];
        Node *parent_node = q.front();

        if (parent_node->l_child == nullptr)
        {
            parent_node->l_child = current_node;
        }
        else if (parent_node->r_child == nullptr)
        {
            parent_node->r_child = current_node;
            q.pop();
        }
        current_node->parent = parent_node; // 設置父節點
        q.push(current_node);
    }
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
        else if (node == node->parent->left)
        {
            int x = node->parent->x + blk_map[node->parent->name]->w;
            node->x = x;
            if (x + blk_map[node->name]->w > x_max)
            {
                x_max = x + blk_map[node->name]->w;
            }
            x_max = max(x + blk_map[node->name]->w, x_max);
            node->y = SetContour_Caly(node);
        }
        else if (node == node->parent->right)
        {
            int x = node->parent->x;
            node->x = x;
           if (x + blk_map[node->name]->w > x_max)
            {
                x_max = x + blk_map[node->name]->w;
            }
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
                continue;
            }
            else if (it->x1 <= x1 && it->x2 <= x2)
            {
                it->x2 = x1;
                continue;
            }
            else
            {
                contour_list.insert(it, Contour(it->x1, x1, it->y));
                it->x1 = x2;
                continue;
            }
        }
    }
    y2 = y1 + blk_map[node->name]->h;
    contour_list.insert(it, Contour(x1, x2, y2));
    if (y2 > y_max)
        y_max = y2;

    return y1;
}
void Floorplanner::Swap_Node(int node1, int node2)
{
    Block *blk1 = blk_map[node_list[node1]->name];
    Block *blk2 = blk_map[node_list[node2]->name];

    blk_map[blk1->name] = blk2;
    blk_map[blk2->name] = blk1;

    node_map[blk1->name] = node_list[node2];
    node_map[blk2->name] = node_list[node1];

    string t_name = blk1->name;
    size_t t_w = blk1->w; // width of the block
    size_t t_h = blk1->h;
    blk1->name = blk2->name;
    blk1->w = blk2->w;
    blk1->h = blk2->h;
    blk2->name = t_name;
    blk2->w = t_w;
    blk2->h = t_h;

    /// string tnname=node_list[node1]->name;
    node_list[node1]->name = node_list[node2]->name;
    node_list[node2]->name = t_name;
}
void Floorplanner::Move_Node(int del, int ins)
{
    ////delete
    if (node_list[del]->left != NULL)
    {
        node_list[del]->left->parent = node_list[del]->parent;
        if (node_list[del]->parent->left == node_list[del])
            node_list[del]->parent->left = node_list[del]->left;
        else
            node_list[del]->parent->right = node_list[del]->left;
        node_list[del]->left = NULL;
    }
    else if (node_list[del]->right != NULL)
    {
        node_list[del]->right->parent = node_list[del]->parent;
        if (node_list[del]->parent->left == node_list[del])
            node_list[del]->parent->left = node_list[del]->right;
        else
            node_list[del]->parent->right = node_list[del]->right;
        node_list[del]->right = NULL;
    }
    else
    {
        if (node_list[del]->parent->left == node_list[del])
            node_list[del]->parent->left = NULL;
        else
            node_list[del]->parent->right = NULL;
    }

    //// insert
    if (node_list[ins]->left == NULL && node_list[ins]->right == NULL)
    {
        if (rand() & 1)
            node_list[ins]->left = node_list[del];
        else
            node_list[ins]->right = node_list[del];
    }
    else if (node_list[ins]->left == NULL)
    {
        node_list[ins]->left = node_list[del];
    }
    else if (node_list[ins]->right == NULL)
    {
        node_list[ins]->right = node_list[del];
    }
    node_list[del]->parent = node_list[ins];
}
void Floorplanner::SA()
{
    BuildTree();
    int reject = 0;
    double T = 100000;
    uniform_real_distribution<> disexp(0.0, 1.0);
    while (1)
    {
        reject = 0;
        for (int i = 0; i < 1000; ++i)
        {
            x_max = 0;
            y_max = 0;
            contour_list.clear();
            SetPos(root);
            double cost = Cal_Cost();
            uniform_int_distribution<> dis(0, 2);
            int op = dis(gen);
            // Rotate
            if (op == 0)
            {
                uniform_int_distribution<> dis(0, num_blk - 1);
                int id = dis(gen);

                size_t temp;
                temp = blk_list[id]->w;
                blk_list[id]->w = blk_list[id]->h;
                blk_list[id]->h = temp;

                x_max = 0;
                y_max = 0;
                contour_list.clear();
                SetPos(root);
                double cost2 = Cal_Cost();

                double prob2 = disexp(gen);
                // double prob2 = (double)rand() / RAND_MAX;
                if (cost < cost2 && prob2 >= exp((double)(cost - cost2) / T))
                {
                    reject++;
                    size_t temp;
                    temp = blk_list[id]->w;
                    blk_list[id]->w = blk_list[id]->h;
                    blk_list[id]->h = temp;
                }
            }
            // Move
            if (op == 1)
            {
                int del_node;
                int ins_node;
                while (true)
                {
                    uniform_int_distribution<> dis(1, num_blk - 1); ////from 1 because not choosing root
                    uniform_int_distribution<> dis2(0, num_blk - 1);
                    del_node = dis(gen);
                    ins_node = dis2(gen);

                    if ((node_list[del_node] != node_list[ins_node]) && (node_list[ins_node]->left == NULL || node_list[ins_node]->right == NULL) && (node_list[del_node]->left == NULL || node_list[del_node]->right == NULL))
                    {
                        break;
                    }
                }

                bool prev_lorr; /// 0 for left, 1 for right
                Node *prevblk = node_list[del_node]->parent;
                if (node_list[del_node]->parent->left == node_list[del_node])
                    prev_lorr = 0;
                else
                    prev_lorr = 1;

                int child_lorr; /// 0 for left, 1 for right, 2 for none
                Node *childblk;
                if (node_list[del_node]->left != NULL)
                {
                    childblk = node_list[del_node]->left;
                    child_lorr = 0;
                }
                else if (node_list[del_node]->right != NULL)
                {
                    childblk = node_list[del_node]->right;
                    child_lorr = 1;
                }
                else
                    child_lorr = 2;

                Move_Node(del_node, ins_node);

                x_max = 0;
                y_max = 0;
                contour_list.clear();
                SetPos(root);
                double cost2 = Cal_Cost();

                double prob2 = disexp(gen);
                // double prob2 = (double)rand() / RAND_MAX;
                if (cost < cost2 && prob2 >= exp((double)(cost - cost2) / T))
                {
                    reject++;
                    if (node_list[ins_node]->left == node_list[del_node])
                        node_list[ins_node]->left = NULL;
                    else
                        node_list[ins_node]->right = NULL;

                    if (prev_lorr == 0)
                    {
                        if (child_lorr == 0)
                        {
                            node_list[del_node]->left = childblk;
                            childblk->parent = node_list[del_node];
                        }
                        else if (child_lorr == 1)
                        {
                            node_list[del_node]->right = childblk;
                            childblk->parent = node_list[del_node];
                        }
                        prevblk->left = node_list[del_node];
                        node_list[del_node]->parent = prevblk;
                    }
                    else if (prev_lorr == 1)
                    {
                        if (child_lorr == 0)
                        {
                            node_list[del_node]->left = childblk;
                            childblk->parent = node_list[del_node];
                        }
                        else if (child_lorr == 1)
                        {
                            node_list[del_node]->right = childblk;
                            childblk->parent = node_list[del_node];
                        }
                        prevblk->right = node_list[del_node];
                        node_list[del_node]->parent = prevblk;
                    }
                }
            }
            // Swap
            if (op == 2)
            {
                uniform_int_distribution<> dis(0, num_blk - 1);
                uniform_int_distribution<> dis2(0, num_blk - 1);
                int fir_node = dis(gen);
                int sec_node = dis2(gen);
                while (fir_node == sec_node)
                {
                    uniform_int_distribution<> dis3(0, num_blk - 1);
                    sec_node = dis3(gen);
                }
                Swap_Node(fir_node, sec_node);
                x_max = 0;
                y_max = 0;
                contour_list.clear();
                SetPos(root);
                double cost2 = Cal_Cost();

                double prob2 = disexp(gen);
                // double prob2 = (double)rand() / RAND_MAX;
                if (cost < cost2 && prob2 >= exp((double)(cost - cost2) / T))
                {
                    reject++;
                    Swap_Node(fir_node, sec_node);
                }
            }
        }
        T *= 0.6;
        double rejectRate = (double)reject / 1000;
        if ((rejectRate > 0.95) && (x_max - outline_width <= 0) && (y_max - outline_height <= 0))
        {
            return;
        }
        if (T < 0.000001 && (x_max - outline_width <= 0) && (y_max - outline_height <= 0))
            return;
    }
}
void Floorplanner::fastSA(int iterNum, double constP, int constK, int constC)
{
  double T, T1;
  double totalDeltaCost; // for calculating deltaCostAvg

  // Fast Simulated Annealing starts
  for (int r = 1; r <= iterNum; ++r)
  {
    if (r == 1)
    {
      T = _deltaAvg / abs(log(constP));
      T1 = T;
    }
    else if (T <= constK)
    {
      T = T1 * totalDeltaCost / (2 * _blockNum + 20) / r / r / constC;
    }
    else
    {
      T = T1 * totalDeltaCost / (2 * _blockNum + 20) / r / r;
    }

    for (int i = 0; i < 2 * _blockNum + 20; ++i)
    {
      // calculate the cost of the original tree
      double oldCost = this->calculateCost(_treeRoot, _blockName2TreeNode);

      // replicate the tree for modification
      _modifiedTreeRoot = this->replicateBStarTree(_treeRoot, nullptr);

      // randomly perturb modified tree
      this->perturb();

      // calculate the cost of the modified tree
      double newCost = this->calculateCost(_modifiedTreeRoot, _blockName2ModifiedTreeNode);

      // calculate the delta cost and probability
      double prob = (double)rand() / (RAND_MAX);
      double deltaCost = newCost - oldCost;
      totalDeltaCost += deltaCost;

      if (deltaCost <= 0)
      {
        // accept the new status and update
        this->deleteTree(_treeRoot);
        _treeRoot = _modifiedTreeRoot;
        _blockName2TreeNode = _blockName2ModifiedTreeNode;
        _blockName2ModifiedTreeNode.clear();
        if (newCost < _bestCost)
        {
          // if the status is the globally best status, update the best status
          // cout << "update at " << r << "th iteration" << endl;
          _bestCost = newCost;
          this->writeBestCoordinateToBlock(_treeRoot);
        }
      }
      else if (1 - prob < exp((-deltaCost) / T))
      {
        // accept the new status but no update best
        this->deleteTree(_treeRoot);
        _treeRoot = _modifiedTreeRoot;
        _blockName2TreeNode = _blockName2ModifiedTreeNode;
        _blockName2ModifiedTreeNode.clear();
      }
      else
      {
        // restore the original status
        this->deleteTree(_modifiedTreeRoot);
        _blockName2ModifiedTreeNode.clear();
      }
    }
  }
}
void Floorplanner::Move_Node(int del, int ins)
{
    // Deletion part
    Node *delNode = node_list[del];
    Node *parentNode = delNode->parent;

    if (delNode->left != NULL)
    {
        delNode->left->parent = parentNode;
        if (parentNode->left == delNode)
            parentNode->left = delNode->left;
        else
            parentNode->right = delNode->left;
    }
    else if (delNode->right != NULL)
    {
        delNode->right->parent = parentNode;
        if (parentNode->left == delNode)
            parentNode->left = delNode->right;
        else
            parentNode->right = delNode->right;
    }
    else
    {
        if (parentNode->left == delNode)
            parentNode->left = NULL;
        else
            parentNode->right = NULL;
    }

    // Clear left and right pointers of the deleted node
    delNode->left = delNode->right = NULL;

    // Insertion part
    Node *insNode = node_list[ins];
    if (insNode->left == NULL && insNode->right == NULL)
    {
        if (rand() & 1)
            insNode->left = delNode;
        else
            insNode->right = delNode;
    }
    else if (insNode->left == NULL)
        insNode->left = delNode;
    else if (insNode->right == NULL)
        insNode->right = delNode;
    // Update the parent pointer
    delNode->parent = insNode;
}

        // copy
        t_node_list.push_back(new Node(name));
        if (t_node_map.count(name) == 0)
            t_node_map[name] = t_node_list[i];