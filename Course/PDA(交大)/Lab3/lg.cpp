#include <iostream>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <tuple>
#include <random>
#include "lg.h"

using namespace std;

// I/O
void Legalizer::Read_lg(ifstream &input)
{
    string str;
    input >> str;
    assert(str == "Alpha");
    input >> alpha;

    input >> str;
    assert(str == "Beta");
    input >> beta;

    input >> str;
    assert(str == "DieSize");
    input >> minX >> minY >> maxX >> maxY;

    string line;
    while (getline(input, line))
    {
        stringstream ss(line);
        string str;
        ss >> str;

        if (str.rfind("FF", 0) == 0)
        {
            string status;
            int x, y, width, height;
            ss >> x >> y >> width >> height >> status;

            Cell *FF = new Cell(str, x, y, width, height, false);
            cells.push_back(FF);
            name2cells[str] = FF;
        }
        else if (str.rfind("C", 0) == 0)
        {
            string status;
            int x, y, width, height;
            ss >> x >> y >> width >> height >> status;

            Cell *cell = new Cell(str, x, y, width, height, true);
            cells.push_back(cell);
            name2cells[str] = cell;
        }
        else if (str == "PlacementRows")
        {
            int x, y, site_width, site_height, num_sites;
            ss >> x >> y >> site_width >> site_height >> num_sites;
            rows.push_back(new Row(x, y, site_width, site_height, num_sites));
        }
    }
    // initialize
    // set boundary
    this->row_h = rows.front()->site_h;
    this->row_sites = rows.front()->num_site;
    this->minX = rows.front()->x;
    this->minY = rows.front()->y;
    this->maxX = rows.back()->x + rows.back()->num_site;
    this->maxY = rows.back()->y + this->row_h;

    //  printCells(cells);
    //  printRows(rows);
    //  cout << "row height = " << row_h << endl;
    // cout << minX << " " << minY << " " << maxX << " " << maxY << endl;

    // put cells on rows
    int start, end;
    int start_x, end_x;
    for (auto &cell : cells)
    {
        start = (cell->y - minY) / this->row_h;
        end = ((cell->y + cell->h - minY) / row_h) - 1;
        start_x = cell->x - minX;
        end_x = cell->x + cell->w - minX;
        // 將 cell 插入對應的 rows 上
        for (int i = start; i <= end; i++)
        {
            rows[i]->cells.push_back(cell);
        }
    }

    // 將每個 Row 的 Cells 根據x由小到大排序
    for (int i = 0; i < rows.size(); i++)
    {
        sort(rows[i]->cells.begin(), rows[i]->cells.end(), CompareX);
    }
    // printRow_cells();
}

void Legalizer::Run_opt(ifstream &input)
{
    string line;
    ofstream output;
    output.open(output_name);
    int tar_x, tar_y;
    tar_x = minX;
    tar_y = minY;
    int i = 0;
    while (getline(input, line))
    {
        // cout << "iter: " << count++ << endl;
        stringstream ss(line);
        string str;
        string FF_name, mFF_name;
        bool result;
        int x, y, w, h;

        // Skip the 'Banking_Cell: ' part
        ss >> str;

        Cell *FF;
        // remove FFs
        while (ss >> FF_name && FF_name != "-->")
        {
            FF = name2cells[FF_name];
            int start = (FF->y - minY) / row_h;             // FF的第一列位在第幾列
            int end = ((FF->y + FF->h - minY) / row_h) - 1; // FF的最後一列位在第幾列
            if ((FF->y + FF->h - minY) % row_h != 0)
                end++;

            // 將 FF 從對應的 rows 上移除
            for (int i = start; i <= end; i++)
            {
                auto &row_cells = rows[i]->cells;                       // 獲取 vector 的引用
                auto it = find(row_cells.begin(), row_cells.end(), FF); // 找到 FF 的迭代器

                if (it != row_cells.end()) // 確認找到該元素
                {
                    row_cells.erase(it); // 使用迭代器刪除元素
                }
                sort(rows[i]->cells.begin(), rows[i]->cells.end(), CompareX);
            }
            name2cells.erase(FF_name);
        }

        // update merged FF
        ss >> mFF_name >> x >> y >> w >> h;
        Cell *mFF = new Cell(mFF_name, x, y, w, h, false);
        cells.push_back(mFF);
        name2cells[mFF_name] = mFF;

        do
        {
            result = MLL(mFF, tar_x, tar_y);
            if (tar_x + mFF->w < maxX)
            {
                tar_x += 10;
            }
            else if (tar_x + mFF->w >= maxX)
            {
                i += 1;
                if (i > rows.size())
                    i = 0;
                tar_x = minX;
                tar_y = minY + i * row_h;
            }
        } while (!result);

        // update and post_lg ...
        tar_x = mFF->x + mFF->w;
        output << mFF->x << " " << mFF->y << endl;
        output << "0" << endl;
    }
}

// local legalization
bool Legalizer::MLL(Cell *mFF, int tar_x, int tar_y)
{
    Cell tFF(mFF->name, mFF->x, mFF->y, mFF->w, mFF->h, false);
    // 檢查並修正 tar_x 和 tar_y
    ClampToBounds(tar_x, mFF->w, minX, maxX);
    ClampToBounds(tar_y, mFF->h, minY, maxY);

    tFF.x = tar_x;
    tFF.y = tar_y;

    Window W(tar_x, tar_y, mFF->w, mFF->h);

    // 檢查並修正 W 的 x 和 y
    if (W.y % this->row_h != 0)
        W.y += this->row_h - W.y % this->row_h; // 確保W的y在row的邊界上
    if (W.h % this->row_h != 0)
        W.h += this->row_h - W.h % this->row_h; // 確保W的upper_y在row的邊界上
    ClampToBounds(W.x, W.w, minX, maxX);
    ClampToBounds(W.y, W.h, minY, maxY);

    int start_W = (W.y - minY) / row_h;           // W的第一列位在第幾列
    int end_W = ((W.y + W.h - minY) / row_h) - 1; // W的最後一列位在第幾列
    bool OK = true;

    for (int j = start_W; j <= end_W; j++)
    {
        for (auto it = rows[j]->cells.begin(); it != rows[j]->cells.end(); it++)
        {
            Cell *cur_cell = *it;
            if (isTouchW(**it, W) && isOverlap(**it, tFF))
            {
                OK = false;
                break;
            }
        }
        if (!OK)
            break;
    }

    if (OK)
    {
        mFF->x = tFF.x;
        mFF->y = tFF.y;
        int start = (mFF->y - minY) / row_h;              // mFF的第一列位在第幾列
        int end = ((mFF->y + mFF->h - minY) / row_h) - 1; // mFF的最後一列位在第幾列
        if ((mFF->y + mFF->h - minY) % row_h != 0)
            end++;
        // 將 mFF 插入在對應的 rows 上
        for (int i = start; i <= end; i++)
        {
            auto pos = std::lower_bound(
                rows[i]->cells.begin(),
                rows[i]->cells.end(),
                mFF,
                CompareX);                   // 找到插入位置
            rows[i]->cells.insert(pos, mFF); // 插入到剛找到的位置
        }

        return true;
    }
    else
    {
        return false;
    }
}

void Legalizer::ClampToBounds(int &pos, int size, int minBound, int maxBound)
{
    if (pos < minBound)
    {
        pos = minBound;
    }
    else if (pos + size > maxBound)
    {
        pos -= (pos + size - maxBound);
    }
}

bool Legalizer::isTouchW(Cell &cell, Window &W)
{
    /*bool Overlap_free = (cell->x + cell->w <= W->x) ||
                        (W->x + W->w <= cell->x) ||
                        (cell->y + cell->h <= W->y) ||
                        (W->y + W->h <= cell->y);*/

    bool Overlap_free = (cell.x + cell.w <= W.x) ||
                        (W.x + W.w <= cell.x) ||
                        (cell.y + cell.h <= W.y) ||
                        (W.y + W.h <= cell.y);

    return !Overlap_free;
}

bool Legalizer::isInW(Cell *cell, Window *W)
{
    return (cell->x >= W->x) &&
           (cell->x + cell->w <= W->x + W->w) &&
           (cell->y >= W->y) &&
           (cell->y + cell->h <= W->y + W->h);
}

bool Legalizer::isOverlap(Cell &a, Cell &b)
{
    bool Overlap_free = (a.x + a.w <= b.x) ||
                        (b.x + b.w <= a.x) ||
                        (a.y + a.h <= b.y) ||
                        (b.y + b.h <= a.y);

    return !Overlap_free;
}

// calculation
int Legalizer::Randx(int k)
{
    // 避免溢位，檢查 Rx * k 是否超過範圍
    if (k > INT32_MAX / Rx)
    {
        k = INT32_MAX / Rx; // 限制 k 的大小
    }
    int min = -Rx * k;
    int max = Rx * k;

    // 確保 rand_x 的範圍計算不會溢位
    int range = max - min;
    if (range < 0)
    {                      // 可能溢位導致負數
        range = INT32_MAX; // 最大可能範圍
    }

    int rand_x = min + rand() % (range + 1);
    return rand_x;
}

int Legalizer::Randy(int k)
{
    // 避免溢位，檢查 Ry * k 是否超過範圍
    if (k > INT32_MAX / Ry)
    {
        k = INT32_MAX / Ry; // 限制 k 的大小
    }
    int min = -Ry * k;
    int max = Ry * k;

    // 確保 rand_y 的範圍計算不會溢位
    int range = max - min;
    if (range < 0)
    {                      // 可能溢位導致負數
        range = INT32_MAX; // 最大可能範圍
    }

    int rand_y = min + rand() % (range + 1);
    return rand_y;
}

bool Legalizer::compareX(Cell &a, Cell &b)
{
    return a.x < b.x;
}

bool Legalizer::CompareX(Cell *a, Cell *b)
{
    return a->x < b->x;
}

Point Legalizer::FindBestPoint(vector<Point> insert_points, Cell &mFF)
{
    int best_cost, cost;
    best_cost = INT32_MAX;
    vector<Point> &pt = insert_points;
    Point best_pt;
    for (int i = 0; i < insert_points.size(); i++)
    {
        cost = abs(mFF.x - pt[i].x) + abs(mFF.y - pt[i].y);
        if (cost < best_cost)
        {
            best_cost = cost;
            best_pt = pt[i];
        }
    }
    return best_pt;
}

// print
void Legalizer::printCells(const vector<Cell *> &cells)
{
    cout << "Printing Cells:" << endl;
    for (const auto &cell : cells)
    {
        cout << cell->name << " "
             << cell->x << " "
             << cell->y << " "
             << cell->w << " "
             << cell->h << " "
             << (cell->isFix ? "Fix" : "NotFix")
             << endl;
    }
    cout << endl;
}

void Legalizer::printRows(const vector<Row *> &rows)
{
    cout << "Printing Rows:" << endl;
    for (const auto &row : rows)
    {
        cout << row->x << " "
             << row->y << " "
             << row->site_w << " "
             << row->site_h << " "
             << endl;
    }
    cout << endl;
}

void Legalizer::printRow_cells()
{
    // 打印每個 Row 的 Cells
    for (size_t i = 0; i < rows.size(); i++)
    {
        Row *row = rows[i];
        cout << "Row " << i << " at position (" << row->x << ", " << row->y << "):" << endl;
        cout << "Cells: ";
        for (size_t j = 0; j < row->cells.size(); ++j)
        {
            cout << row->cells[j]->x << " ";
        }
        cout << endl;
    }
}