#ifndef TILE_H_
#define TILE_H_
#define X first
#define Y second
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <utility>
#include <algorithm> 
#include <functional>
using namespace std;

enum TileType{solid, space};
class Point 
{
 public:
    int x, y;
    Point() = default; 
    Point(int a, int b) : x(a), y(b) {}
};
class Tile
{
 public:
    TileType type; //solid or space tile
    int index;
    //int block_neighbor_count, space_neighbor_count;
    Point left_bottom, right_top; 
    Tile *rt, *tr, *lb, *bl;
    Tile() = default;
    Tile(TileType type, Point LB, Point RT);
  
    bool IsInTile(Point pt); //if the point is in the tile
    void SetPtr (Tile* new_rt, Tile* new_tr, Tile* new_lb, Tile* new_bl);
    void TileInfo();
    void Neighbor_TileInfo();
};

Tile::Tile(TileType tile_type, Point LB, Point RT) : type(tile_type), left_bottom(LB), right_top(RT)
{
    rt = nullptr;
    tr = nullptr;
    lb = nullptr;
    bl = nullptr;
    if (tile_type == space) index = -1;
}
bool Tile::IsInTile(Point pt)
{
    return pt.x >= left_bottom.x && pt.y >= left_bottom.y
            &&pt.x < right_top.x && pt.y < right_top.y;
}
void Tile::SetPtr (Tile* new_rt, Tile* new_tr, Tile* new_lb, Tile* new_bl)
{
    rt = new_rt;
    tr = new_tr;
    lb = new_lb;
    bl = new_bl;
}
void Tile::TileInfo()
{
    if (this == nullptr) cout << "NULL" << endl;
    else{cout << index << "("<<left_bottom.x<<","<<left_bottom.y<<") , ("<<right_top.x<<","<<right_top.y<<")\n";}
}
void Tile::Neighbor_TileInfo()
{
    if (rt)
        cout << "rt tile: ", rt->TileInfo();
    if (tr)
        cout << "tr tile: ", tr->TileInfo();
    if (lb)
        cout << "lb tile: ", lb->TileInfo();
    if (bl)
        cout << "bl tile: ", bl->TileInfo();
}
#endif