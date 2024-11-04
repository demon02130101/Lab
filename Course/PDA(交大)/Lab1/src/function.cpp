#include "function.h"
using namespace std;

Tile* Layout::NewTile(TileType type, Point LB, Point RT)
{
    Tile* new_tile = new Tile(type, LB, RT);
    tile_count++;
    return new_tile;
}
void Layout::Initial()
{
    tile_count = 0;
    start_tile = NewTile(space, Point(0,0),Point(outline_width, outline_height));
}

Tile* Layout::PointFinding(Point target)
{
    Tile *tilePtr = start_tile;
    //cout<<"before: ";tilePtr->TileInfo();
    while (tilePtr && !tilePtr->IsInTile(target))
    {
        //vertical move
        while ( tilePtr && !(tilePtr->right_top.y > target.y && tilePtr->left_bottom.y <= target.y) )
        {
            if (tilePtr->right_top.y <= target.y)
                tilePtr = tilePtr->rt;
            else
                tilePtr = tilePtr->lb;
        }
        //horizontal move
        while ( tilePtr && !(tilePtr->right_top.x > target.x && tilePtr->left_bottom.x <= target.x) )
        {
            if (tilePtr->right_top.x <= target.x)
                tilePtr = tilePtr->tr;
            else
                tilePtr = tilePtr->bl;
        }
    }
    //it may raise up the speed of Point Finding after
    if (tilePtr) 
        start_tile = tilePtr;
    //cout<<"after: ";tilePtr->TileInfo();
    return tilePtr;
}
vector<Tile*> Layout::NeighborFindingTop(Tile* tile)
{
    vector<Tile*> neighbors;
    Tile* tilePtr = tile->rt;

    while (tilePtr && !(tilePtr->right_top.x <= tile->left_bottom.x)) 
    {
        neighbors.push_back(tilePtr);
        tilePtr = tilePtr->bl;
    }
    return neighbors;
}
vector<Tile*> Layout::NeighborFindingBottom(Tile* tile)
{
    vector<Tile*> neighbors;
    Tile* tilePtr = tile->lb;

    while (tilePtr && !(tilePtr->left_bottom.x >= tile->right_top.x)) 
    {
        neighbors.push_back(tilePtr);
        tilePtr = tilePtr->tr;
    }
    return neighbors;
}
vector<Tile*> Layout::NeighborFindingLeft(Tile* tile)
{
    vector<Tile*> neighbors;
    Tile* tilePtr = tile->bl;

    while (tilePtr && !(tilePtr->left_bottom.y >= tile->right_top.y)) 
    {
        neighbors.push_back(tilePtr);
        tilePtr = tilePtr->rt;
    }
    return neighbors;
}
vector<Tile*> Layout::NeighborFindingRight(Tile* tile)
{
    vector<Tile*> neighbors;
    Tile* tilePtr = tile->tr;
      
    while (tilePtr && !(tilePtr->right_top.y <= tile->left_bottom.y)) 
    {
        neighbors.push_back(tilePtr);
        tilePtr = tilePtr->lb;
    }
    return neighbors;
}
vector<Tile*> Layout::GetNeighbors(Tile* tile)
{
    vector<Tile*> neighbors;
    vector<Tile*> top = NeighborFindingTop(tile);
    for (Tile* t : top) 
    {
        neighbors.push_back(t);
    }    

    vector<Tile*> bottom = NeighborFindingBottom(tile);
    for (Tile* b : bottom) 
    {
        neighbors.push_back(b);
    }    

    vector<Tile*> left = NeighborFindingLeft(tile);
    for (Tile* l : left) 
    {
        neighbors.push_back(l);
    }

    vector<Tile*> right = NeighborFindingRight(tile);
    for (Tile* r : right) 
    {
        neighbors.push_back(r);
    }

    return neighbors;
}

bool Layout::AreaSearch(Point area_left_bottom, Point area_right_top)
{
    Point start_point(area_left_bottom.x, area_right_top.y);
    Tile* tilePtr = PointFinding(start_point);
    //point on the outline that is not belong to any tile
    if (!tilePtr) 
        tilePtr = PointFinding(Point(area_left_bottom.x, area_right_top.y-1));
    //the found tile just touch the top edge of area
    if (tilePtr->left_bottom.y == area_right_top.y)        
        tilePtr = PointFinding(Point(area_left_bottom.x, tilePtr->left_bottom.y-1));
    //loop until reach the bottom of area
    while (tilePtr && !(tilePtr->right_top.y <= area_left_bottom.y))  
    {
        if (tilePtr->type == solid || tilePtr->right_top.x < area_right_top.x) //there is a solid tile on the right side.
            return true;
        tilePtr = tilePtr->lb;
        while (tilePtr && !(tilePtr->right_top.x > area_left_bottom.x)) //tilePtr->lb is out of the area
            tilePtr = tilePtr->tr;           
    }
    return false;
}
bool Layout::TileCreation(int index, Point tile_left_bottom, Point tile_right_top)
{
    if (AreaSearch(tile_left_bottom, tile_right_top)) 
    {
        cout << "Warning:The area already has a solid tile." << endl;
        return false;
    }
    //Step1:Find the space tile containing the top edge of target area
    Point top_point(tile_left_bottom.x, tile_right_top.y);
    Tile* top_spaceTile = PointFinding(top_point);
    if (!top_spaceTile) //target is on the top outline
        top_spaceTile = PointFinding(Point(tile_left_bottom.x, tile_right_top.y-1));
   
    if (tile_right_top.y == top_spaceTile->left_bottom.y) //top edge is not belong to the area
    {
        top_point.y--;
        top_spaceTile = PointFinding(top_point);
    }
    //Split the top space tile along a horizontal line
    Tile* new_top_spaceTile = (!On_TopEdge(top_spaceTile, tile_left_bottom, tile_right_top))
                             ? SplitTop(top_spaceTile, tile_left_bottom, tile_right_top)
                             : PointFinding(Point(tile_left_bottom.x-1, tile_right_top.y));

    //Step2:Find the space tile containing the bottom edge of target area
    Tile* bottom_spaceTile = PointFinding(tile_left_bottom);
    //Split the bottom space tile along the horizontal line
    Tile* new_bottom_spaceTile = (!On_BottomEdge(bottom_spaceTile, tile_left_bottom, tile_right_top))
                                   ? SplitBottom(bottom_spaceTile, tile_left_bottom, tile_right_top)
                                   : nullptr;

    //Step3:Split the center space tile
    Point current_point = top_point;
    Tile* center_spaceTile = top_spaceTile;
    Tile* next_tile = nullptr;
    Tile* next_of_last_spaceTile = PointFinding(Point(tile_left_bottom.x, tile_left_bottom.y-1));
    Tile* new_block = NewTile(solid, tile_left_bottom, tile_right_top);
    vector<Tile*> center_space_list;

    while (center_spaceTile != next_of_last_spaceTile) 
    {   
        current_point.y = center_spaceTile->left_bottom.y-1;
        next_tile = PointFinding(Point(current_point)); 
        SplitCenter(center_spaceTile, new_block, top_spaceTile, bottom_spaceTile, new_top_spaceTile);
        center_spaceTile = next_tile;  
    }

    //Step4:Merge space tiles
    vector<Tile*> right_tiles = NeighborFindingRight(new_block);
    vector<Tile*> left_tiles = NeighborFindingLeft(new_block);

    if (!right_tiles.empty())
        MergeRight(right_tiles);
    if (!left_tiles.empty()) 
        MergeLeft(left_tiles);

    //Update the block_list
    new_block->index = index;
    block_list.push_back(new_block);
    //cout << "Block: "; new_block->TileInfo(); new_block->Neighbor_TileInfo();
    return true;
}
bool Layout::On_TopEdge(Tile* top_spaceTile, Point area_left_bottom, Point area_right_top)
{
    return area_right_top.y == top_spaceTile->right_top.y
        || area_right_top.y == top_spaceTile->left_bottom.y;
}
bool Layout::On_BottomEdge(Tile* bottom_spaceTile, Point area_left_bottom, Point area_right_top)
{
    return area_left_bottom.y == bottom_spaceTile->right_top.y 
        || area_left_bottom.y == bottom_spaceTile->left_bottom.y;
}
void Layout::TileDeletion(Tile* tile)
{
    //Update start_tile if need
    if (tile == start_tile) 
    {
        if (start_tile->rt)  
            start_tile = start_tile->rt;
        else if (start_tile->tr) 
            start_tile = start_tile->tr;
        else if (start_tile->lb) 
            start_tile = start_tile->lb;
        else if (start_tile->bl) 
            start_tile = start_tile->bl;
        else 
            cout << "Warning: start_tile will be killed!!!" << endl;    
    }
    delete tile;
    tile_count--;    
}
void Layout::SortBlock()
{
    sort(block_list.begin(), block_list.end(), [](const Tile* t1, const Tile* t2) 
    {//sort the blocks in increasing order according to their index
        return t1->index < t2->index;
    });
}