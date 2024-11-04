#ifndef FUNCTION_H_
#define FUNCTION_H_
#include "tile.h"
using namespace std;

class Layout
{
 private:
    Tile* NewTile(TileType type, Point left_bottom, Point right_top);
    bool On_TopEdge(Tile* top_space, Point left_bottom, Point right_top);
    bool On_BottomEdge(Tile* bottom_space, Point left_bottom, Point right_top);
    Tile* SplitTop(Tile* top_space, Point left_bottom, Point right_top); 
    Tile* SplitBottom(Tile* bottom_space, Point left_bottom, Point right_top);
    void SplitCenter(Tile* center_space, Tile* new_block, Tile* top_space, Tile* bottom_space, Tile*& rt_of_leftTile);
    bool MayMergeable(Tile* t1,Tile* t2);
    bool IsMergeable(Tile* t1,Tile* t2);
    void SetPtr_Merge(Tile* curr, Tile* next, Tile* (Tile::*dir1), Tile* (Tile::*dir2), Tile* (Tile::*dir3));
    void SetPtr_Merge(Tile* curr, Tile* new_block, Tile* (Tile::*dir1), Tile* (Tile::*dir2), Tile* (Tile::*dir3), Tile* bound_tile);
    void MergeLeft(vector<Tile*> left_tiles);
    void MergeRight(vector<Tile*> right_tiles);
    void TileDeletion(Tile* tile); 

 public:
    vector<Tile*> block_list;
    vector<Point> pointFind_list; 
    int tile_count; //include solid and space tiles
    int outline_width, outline_height;
    Tile* start_tile;
    Layout() = default;

    void Initial();
    void ReadFile(const string input_filename);//not yet!!!
    void OutputFile(const string output_filename);//not yet!!!
    Tile* PointFinding(Point target);
    vector<Tile*> NeighborFindingTop(Tile* tile);
    vector<Tile*> NeighborFindingBottom(Tile* tile);
    vector<Tile*> NeighborFindingLeft(Tile* tile);
    vector<Tile*> NeighborFindingRight(Tile* tile);
    vector<Tile*> GetNeighbors(Tile* tile);    
    bool AreaSearch(Point area_left_bottom, Point area_right_top); //if the area has any solid tile
    bool TileCreation(int index, Point tile_left_bottom, Point tile_right_top);
    void SortBlock();
};

#endif