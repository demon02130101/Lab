bool isOverlap()
{
}

bool tryInsertCell(Window &window, Cell &cell)
{
    // 檢查是否與 window 中的其他 cell 重疊
    for (auto &existing_cell : window.loc_cells)
    {
        if (isOverlap(*existing_cell, cell))
        {
            return false; // 有重疊，插入失敗
        }
    }
    // 無重疊，插入成功
    window.loc_cells.push_back(new Cell(cell));
    return true;
}

// 將每個 Row 的 Cells 根據x由小到大排序
for (int i = 0; i < rows.size(); i++)
{
    sort(rows[i]->cells.begin(), rows[i]->cells.end(), CompareX);
}

bool Legalizer::MLL(Cell *mFF, int tar_x, int tar_y)
{
    Cell *tFF = new Cell(mFF->name, mFF->x, mFF->y, mFF->w, mFF->h, false); // temp
    // 檢查並修正 tar_x 和 tar_y
    if (tar_y % this->row_h != 0)
        tar_y += this->row_h - tar_y % this->row_h; // 確保tar_y在row的邊界上
    ClampToBounds(tar_x, mFF->w, minX, maxX);
    ClampToBounds(tar_y, mFF->h, minY, maxY);
    // 更新 temp mFF 座標
    tFF->x = tar_x;
    tFF->y = tar_y;

    Window *W = new Window(tar_x - Rx, tar_y - Ry, 2 * Rx + mFF->w, 2 * Ry + mFF->h);

    // 檢查並修正 W 的 x 和 y
    if (W->y % this->row_h != 0)
        W->y += this->row_h - W->y % this->row_h; // 確保W的y在row的邊界上
    if (W->h % this->row_h != 0)
        W->h += this->row_h - W->h % this->row_h; // 確保W的upper_y在row的邊界上
    ClampToBounds(W->x, W->w, minX, maxX);
    ClampToBounds(W->y, W->h, minY, maxY);

    vector<Cell> W_cells;
    int start = (W->y - minY) / row_h;            // W的第一列位在第幾列
    int end = ((W->y + W->h - minY) / row_h) - 1; // W的最後一列位在第幾列
    bool Overlap = false;

    for (int i = start; i <= end; i++)
    {
        for (auto it = rows[i]->cells.begin(); it != rows[i]->cells.end(); it++)
        {
            if (isTouchW(*it, W))
            {
                W_cells.push_back(**it);
            }
        }
    }

    // find all valid insertion points
    vector<int> insert_points;
    for (int i = 1; i < W_cells.size(); i++)
    {
        int xL = W_cells[i - 1].x + W_cells[i - 1].w;
        int xR = W_cells[i].x;
        int gap = xR - xL;
        if (gap >= mFF->w)
            insert_points.push_back(xL);
    }

    // find best point from above
    FindBestPoint(insert_points, *mFF);

    if (Success)
    {
        mFF->x = point;
        mFF->y = tar_y;
        int start = (mFF->y - minY) / row_h;              // mFF的第一列位在第幾列
        int end = ((mFF->y + mFF->h - minY) / row_h) - 1; // mFF的最後一列位在第幾列
        if ((mFF->y + mFF->h - minY) % row_h != 0)
            end++;

        // 將 mFF 插入在對應的 rows 上
        for (int i = start; i <= end; i++)
        {
            rows[i]->cells.push_back(mFF);
            sort(rows[i]->cells.begin(), rows[i]->cells.end(), CompareX);
        }

        // release memory

        delete W;

        return true;
    }
    else
    {
        // release memory

        delete W;

        return false;
    }
}

void Legalizer::FindGaps(const vector<vector<Cell>> &W_cells, vector<vector<Gap>> &gaps, const Window &W, const Cell &tFF)
{
    gaps.resize(W_cells.size());

    for (size_t i = 0; i < W_cells.size(); ++i)
    {
        const vector<Cell> &rowCells = W_cells[i]; // 第 i 行的 cells
        vector<Gap> &rowGaps = gaps[i];            // 第 i 行的 gaps
        rowGaps.clear();                           // 清空當前行的 gaps

        int WL = W.x;       // Window 左邊界
        int WR = W.x + W.w; // Window 右邊界

        // 1. 檢查 Window 左邊界到第一個 Cell 的 gap
        if (!rowCells.empty() && rowCells.front().x > WL)
        {
            if (rowCells.front().x - WL >= tFF.w)
                rowGaps.emplace_back(WL, rowCells.front().x);
        }

        // 2. 檢查相鄰 Cell 之間的 gaps
        for (size_t j = 1; j < rowCells.size(); j++)
        {
            int prevR = rowCells[j - 1].x + rowCells[j - 1].w; // 前一個 Cell 的右邊界
            int currL = rowCells[j].x;                         // 當前 Cell 的左邊界

            if (currL - prevR >= tFF.w)
                rowGaps.emplace_back(prevR, currL);
        }

        // 3. 檢查最後一個 Cell 到 Window 右邊界的 gap
        if (!rowCells.empty() && rowCells.back().x + rowCells.back().w < WR)
        {
            if (WR - (rowCells.back().x + rowCells.back().w) >= tFF.w)
                rowGaps.emplace_back(rowCells.back().x + rowCells.back().w, WR);
        }

        // 4. 如果整行沒有 Cell，整行都是一個 gap
        if (rowCells.empty())
        {
            rowGaps.emplace_back(WL, WR);
        }
        /*cout << "row " << i << " ";
        for (auto it = gaps[i].begin(); it != gaps[i].end(); it++)
        {
            cout << it->xL << " ";
        }
        cout << endl;
        */
    }
}

void Print()
{
    // 顯示 insert points
    for (int i = 0; i < insert_points.size(); i++)
    {
        cout << insert_points[i] << " ";
    }
    cout << endl;

    for (int i = 0; i < W_cells.size(); i++)
    {
        cout << W_cells[i].name << " "
             << W_cells[i].x << " "
             << W_cells[i].y << endl;
    }
}

bool Legalizer::MLL(Cell *mFF, int tar_x, int tar_y)
{
    Cell *tFF = new Cell(mFF->name, mFF->x, mFF->y, mFF->w, mFF->h, false);
    // 檢查並修正 tar_x 和 tar_y
    if (tar_y % this->row_h != 0)
        tar_y -= tar_y % this->row_h; // 確保tar_y在row的邊界上
    ClampToBounds(tar_x, mFF->w, minX, maxX);
    ClampToBounds(tar_y, mFF->h, minY, maxY);

    tFF->x = tar_x;
    tFF->y = tar_y;

    Window *W = new Window(tar_x - Rx, tar_y - Ry, 2 * Rx + mFF->w, 2 * Ry + mFF->h);

    // 檢查並修正 W 的 x 和 y
    if (W->y % this->row_h != 0)
        W->y += this->row_h - W->y % this->row_h; // 確保W的y在row的邊界上
    if (W->h % this->row_h != 0)
        W->h += this->row_h - W->h % this->row_h; // 確保W的upper_y在row的邊界上
    ClampToBounds(W->x, W->w, minX, maxX);
    ClampToBounds(W->y, W->h, minY, maxY);

    vector<Cell> W_cells;
    // bool Overlap = false;
    int start_W = (W->y - minY) / row_h;            // W的第一列位在第幾列
    int end_W = ((W->y + W->h - minY) / row_h) - 1; // W的最後一列位在第幾列
    bool OK = true;
    for (int i = start_W; i <= end_W; i++)
    {
        for (auto it = rows[i]->cells.begin(); it != rows[i]->cells.end(); it++)
        {
            if (isTouchW(*it, W))
            {
                W_cells.push_back(**it);
                if (isOverlap(**it, *tFF))
                {
                    // Overlap = true;
                    OK = false;
                }
            }
        }
    }
    
    // step1: find all possible insertion points x座標
    sort(W_cells.begin(), W_cells.end(), compareX);
    vector<int> insert_x;
    int WL = W->x;        // Window 左邊界
    int WR = W->x + W->w; // Window 右邊界
    // cout << "W " << WL << " " << WR << " " << W->y << endl;

    // 1. 檢查 Window 左邊界到第一個 Cell 的 gap
    int gap;
    if (!W_cells.empty() && W_cells.front().x > WL)
    {
        gap = W_cells.front().x - WL;
        if (gap >= mFF->w)
        {
            insert_x.push_back(WL);
        }
    }
    // 2. 檢查相鄰 Cell 之間的 gaps
    for (int j = 1; j < W_cells.size(); j++)
    {
        int prevR = W_cells[j - 1].x + W_cells[j - 1].w; // 前一個 Cell 的右邊界
        int currL = W_cells[j].x;                        // 當前 Cell 的左邊界
        gap = currL - prevR;
        if (gap >= mFF->w)
        {
            insert_x.push_back(prevR);
        }
    }
    // 3. 檢查最後一個 Cell 到 Window 右邊界的 gap
    if (!W_cells.empty() && W_cells.back().x + W_cells.back().w < WR)
    {
        gap = WR - (W_cells.back().x + W_cells.back().w);
        if (gap >= mFF->w)
        {
            insert_x.push_back(W_cells.back().x + W_cells.back().w);
        }
    }
    // 4. 如果沒有 Cell，整列都是一個 gap
    if (W_cells.empty())
    {
        if (WR - WL >= mFF->w)
        {
            insert_x.push_back(WL);
        }
    }

    // step2: find all valid insertion points y座標
    vector<Point> insert_points;
    bool Success = false;
    bool Overlap = false;
    if (!insert_x.empty())
    {
        int W_row_num = W->h / this->row_h;
        for (int i = 0; i < insert_x.size(); i++)
        {
            tFF->x = insert_x[i];
            for (int j = 0; j < W_row_num; j++)
            {
                tFF->y = W->y + j * this->row_h;
                int end_FF = (tFF->y + tFF->h - minY) / row_h - 1;
                int err_h = end_FF + 1 - rows.size();
                for (int k = 0; k < W_cells.size(); k++)
                {
                    if (isOverlap(*tFF, W_cells[k]))
                    {
                        // cout << W_cells[k].name << endl;
                        Overlap = true;
                    }
                }
                if (!Overlap && err_h < 0)
                {
                    insert_points.push_back(Point(tFF->x, tFF->y));
                    Success = true;
                    Overlap = false;
                }
            }
        }
    }

    // find best point from above
    Point best_pt;
    if (!insert_points.empty())
    {
        best_pt = FindBestPoint(insert_points, *mFF);
    }

    if (OK)
    {
        // tar_x = best_pt.x;
        // tar_y = best_pt.y;

        mFF->x = tar_x;
        mFF->y = tar_y;
        int start = (mFF->y - minY) / row_h;              // mFF的第一列位在第幾列
        int end = ((mFF->y + mFF->h - minY) / row_h) - 1; // mFF的最後一列位在第幾列
        if ((mFF->y + mFF->h - minY) % row_h != 0)
            end++;
        // 將 mFF 插入在對應的 rows 上
        for (int i = start; i <= end; i++)
        {
            rows[i]->cells.push_back(mFF);
            sort(rows[i]->cells.begin(), rows[i]->cells.end(), CompareX);
        }

        // release memory
        delete W;
        delete tFF;

        return true;
    }
    else
    {
        // release memory
        delete W;
        delete tFF;

        return false;
    }
}
