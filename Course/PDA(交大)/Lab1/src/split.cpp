#include "function.h"
using namespace std;

Tile* Layout::SplitTop(Tile* top_spaceTile, Point left_bottom, Point right_top)
{
    Tile* new_tile = NewTile(space, Point(top_spaceTile->left_bottom.x, right_top.y), Point(top_spaceTile->right_top.x, top_spaceTile->right_top.y));
    //update new_tile->lb
    new_tile->SetPtr(top_spaceTile->rt, top_spaceTile->tr, top_spaceTile, top_spaceTile->bl);
    //update new_tile->bl
    while (new_tile->bl && new_tile->bl->right_top.y <= new_tile->left_bottom.y)
        new_tile->bl = new_tile->bl->rt;

    //fix the tiles whose lb = top_space 
    Tile *temp_tile = top_spaceTile->rt;
    while(temp_tile && temp_tile->lb == top_spaceTile)
    {
        temp_tile->lb = new_tile;
        temp_tile = temp_tile->bl;
    }
    //fix the tiles whose tr = top_space 
    temp_tile = top_spaceTile->bl;
    while (temp_tile && temp_tile->tr == top_spaceTile)
    {
        if(temp_tile->right_top.y > new_tile->left_bottom.y)
            temp_tile->tr = new_tile;
        temp_tile = temp_tile->rt;
    }
    //fix the tiles whose bl = top_space 
    temp_tile = top_spaceTile->tr;
    while(temp_tile && temp_tile->bl == top_spaceTile)
    {
        if(temp_tile->left_bottom.y >= new_tile->left_bottom.y)
            temp_tile->bl = new_tile;
        temp_tile = temp_tile->lb;
    }
    //update top_space->rt
    top_spaceTile->rt = new_tile;
    top_spaceTile->right_top.y = right_top.y;
    //update top_space->tr
    while (top_spaceTile->tr && top_spaceTile->tr->left_bottom.y >= top_spaceTile->right_top.y)
        top_spaceTile->tr = top_spaceTile->tr->lb;
    return new_tile;    
}
Tile* Layout::SplitBottom(Tile* bottom_spaceTile, Point left_bottom, Point right_top)
{
    Tile* new_tile = NewTile(space, Point(bottom_spaceTile->left_bottom.x, bottom_spaceTile->left_bottom.y), Point(bottom_spaceTile->right_top.x, left_bottom.y));
    //update new_tile->rt
    new_tile->SetPtr(bottom_spaceTile, bottom_spaceTile->tr, bottom_spaceTile->lb, bottom_spaceTile->bl);

    //update new_tile->tr
    while(new_tile->tr && new_tile->tr->left_bottom.y >= new_tile->right_top.y)
        new_tile->tr = new_tile->tr->lb;
   
    //fix the tiles whose rt = bottom_space
    Tile *temp_tile = bottom_spaceTile->lb;
    while(temp_tile && temp_tile->rt == bottom_spaceTile)
    {
        temp_tile->rt = new_tile;
        temp_tile = temp_tile->tr;
    }
    //fix the tiles whose tr = bottom_space
    temp_tile = bottom_spaceTile->bl;
    while (temp_tile && temp_tile->tr == bottom_spaceTile)
    {
        if(temp_tile->right_top.y <= new_tile->right_top.y)
            temp_tile->tr = new_tile;
        temp_tile = temp_tile->rt;
    }
    //fix the tiles whose bl = bottom_space
    temp_tile = bottom_spaceTile->tr;
    while(temp_tile && temp_tile->bl == bottom_spaceTile)
    {
        if(temp_tile->left_bottom.y < new_tile->right_top.y)
            temp_tile->bl = new_tile;
        temp_tile = temp_tile->lb;
    }
    //update bottom_space->lb
    bottom_spaceTile->lb = new_tile;
    bottom_spaceTile->left_bottom.y = left_bottom.y;
    //update bottom_space->bl
    while (bottom_spaceTile->bl && bottom_spaceTile->bl->right_top.y <= bottom_spaceTile->left_bottom.y)
        bottom_spaceTile->bl = bottom_spaceTile->bl->rt;
    return new_tile;
}
void Layout::SplitCenter(Tile* center_spaceTile, Tile* new_block, Tile* top_space_tile, Tile* bottom_space_tile,Tile*& new_top_spaceTile)
{
    bool no_left_space = center_spaceTile->left_bottom.x == new_block->left_bottom.x;
    bool no_right_space = center_spaceTile->right_top.x == new_block->right_top.x;
    Tile* temp_tile;

    if (no_left_space && no_right_space) 
    {
        // Update pointers in all four directions 
        SetPtr_Merge(center_spaceTile, new_block, &Tile::bl, &Tile::tr, &Tile::rt); 
        SetPtr_Merge(center_spaceTile, new_block, &Tile::tr, &Tile::bl, &Tile::lb); 
        SetPtr_Merge(center_spaceTile, new_block, &Tile::rt, &Tile::lb, &Tile::bl); 
        SetPtr_Merge(center_spaceTile, new_block, &Tile::lb, &Tile::rt, &Tile::tr, bottom_space_tile);

        // Update new_block's neighbor 
        if (center_spaceTile == top_space_tile) 
        {
            new_block->tr = center_spaceTile->tr;
            new_block->rt = center_spaceTile->rt;
        }
        // Update new_block's neighbor 
        if (center_spaceTile == bottom_space_tile) 
        {
            new_block->bl = center_spaceTile->bl;
            new_block->lb = center_spaceTile->lb;
        }
        new_top_spaceTile = center_spaceTile->bl;
        TileDeletion(center_spaceTile);
    } 
    else if (no_right_space) 
    {
        // Update the tiles whose bl = center_space
        SetPtr_Merge(center_spaceTile, new_block, &Tile::tr, &Tile::bl, &Tile::lb); 
        // Update the tiles whose rt = center_space
        temp_tile = center_spaceTile->lb;
        while (temp_tile && temp_tile->rt == center_spaceTile) 
        {
            if (temp_tile->right_top.x > new_block->left_bottom.x &&
                temp_tile->right_top.x <= new_block->right_top.x) 
            {
                if (center_spaceTile == bottom_space_tile) 
                    temp_tile->rt = new_block;
                else 
                    temp_tile->rt = nullptr;
            }
            temp_tile = temp_tile->tr;
        }
        // Update the tiles whose lb = center_space
        temp_tile = center_spaceTile->rt;
        while (temp_tile && temp_tile->lb == center_spaceTile) 
        {
            if (temp_tile->left_bottom.x >= new_block->left_bottom.x) 
            {
                temp_tile->lb = new_block;
            }
            temp_tile = temp_tile->bl;
        }
        // Update new_block's neighbor 
        if (center_spaceTile == top_space_tile) 
        {
            new_block->tr = center_spaceTile->tr;
            new_block->rt = center_spaceTile->rt;
        }
        // Update new_block's neighbor 
        if (center_spaceTile == bottom_space_tile) 
        {
            new_block->bl = center_spaceTile;
            temp_tile = center_spaceTile->lb;
            while (temp_tile && temp_tile->left_bottom.x <= new_block->left_bottom.x) 
            {
                if (temp_tile->right_top.x > new_block->left_bottom.x) 
                {
                    new_block->lb = temp_tile;
                }
                temp_tile = temp_tile->tr;
            }
        }
        center_spaceTile->right_top.x = new_block->left_bottom.x;
        center_spaceTile->tr = new_block;
        center_spaceTile->rt = new_top_spaceTile;
        new_top_spaceTile = center_spaceTile;
    } 
    else if (no_left_space) 
    {
        // Update the tiles whose tr = center_space
        SetPtr_Merge(center_spaceTile, new_block, &Tile::bl, &Tile::tr, &Tile::rt); 
        // Update the tiles whose rt = center_space
        temp_tile = center_spaceTile->lb;
        while (temp_tile && temp_tile->rt == center_spaceTile) 
        {
            if (temp_tile->right_top.x <= new_block->right_top.x) 
            {
                if (center_spaceTile == bottom_space_tile) 
                    temp_tile->rt = new_block;
                else 
                    temp_tile == nullptr;
            } 
            else if ( temp_tile->right_top.x > new_block->right_top.x &&
                     temp_tile->right_top.x <= center_spaceTile->right_top.x) 
                     temp_tile->rt = center_spaceTile;
            temp_tile = temp_tile->tr;
        }
        // Update the tiles whose lb = center_space
        temp_tile = center_spaceTile->rt;
        while (temp_tile && temp_tile->lb == center_spaceTile) 
        {
            if (temp_tile->left_bottom.x < new_block->right_top.x) 
                temp_tile->lb = new_block;
            temp_tile = temp_tile->bl;
        }
        // Update new_block's neighbor 
        if (center_spaceTile == top_space_tile) 
        {
            new_block->tr = center_spaceTile;
            temp_tile = center_spaceTile->rt;
            while (temp_tile && temp_tile->right_top.x >= new_block->right_top.x) 
            {
                if (temp_tile->left_bottom.x < new_block->right_top.x) 
                    new_block->rt = temp_tile;
                temp_tile = temp_tile->bl;
            }
        }
        // Update new_block's neighbor 
        if (center_spaceTile == bottom_space_tile) 
        {
            new_block->bl = center_spaceTile->bl;
            new_block->lb = center_spaceTile->lb;
        }
        new_top_spaceTile = center_spaceTile->bl;
        center_spaceTile->left_bottom.x = new_block->right_top.x;
        center_spaceTile->bl = new_block;
        while (center_spaceTile->lb && center_spaceTile->lb->right_top.x <= center_spaceTile->left_bottom.x) 
            center_spaceTile->lb = center_spaceTile->lb->tr;
    } 
    else // have both left and right sides
    {
        Tile* new_right_space_tile = NewTile(space, Point(new_block->right_top.x, center_spaceTile->left_bottom.y), center_spaceTile->right_top);
        // Update ->bl
        new_right_space_tile->SetPtr(center_spaceTile->rt, center_spaceTile->tr, center_spaceTile->lb, new_block);

        while (new_right_space_tile->lb && new_right_space_tile->lb->right_top.x <= new_right_space_tile->left_bottom.x) 
            new_right_space_tile->lb = new_right_space_tile->lb->tr;

        // Update the tiles whose bl = middle_space
        temp_tile = center_spaceTile->tr;
        while (temp_tile && temp_tile->bl == center_spaceTile) 
        {
            temp_tile->bl = new_right_space_tile;
            temp_tile = temp_tile->lb;
        }

        // Update the tiles whose rt = middle_space 
        temp_tile = center_spaceTile->lb;
        while (temp_tile && temp_tile->rt == center_spaceTile) 
        {
            if (temp_tile->right_top.x > new_block->right_top.x) 
                temp_tile->rt = new_right_space_tile;
            else if (temp_tile->right_top.x > new_block->left_bottom.x) 
            {
                if (center_spaceTile == bottom_space_tile) 
                    temp_tile->rt = new_block;
                else 
                    temp_tile->rt = nullptr;
            }
            temp_tile = temp_tile->tr;
        }

        // Update the tiles whose lb = middle_space
        temp_tile = center_spaceTile->rt;
        while (temp_tile && temp_tile->lb == center_spaceTile) 
        {
            if (temp_tile->left_bottom.x >= new_block->right_top.x) 
                temp_tile->lb = new_right_space_tile;
            else if (temp_tile->left_bottom.x >= new_block->left_bottom.x) 
            {
                if (center_spaceTile == top_space_tile) 
                    temp_tile->lb = new_block;
                else 
                    temp_tile->lb = nullptr;
            }
            temp_tile = temp_tile->bl;
        }

        // Update new_block's neighbor (tr, rt)
        if (center_spaceTile == top_space_tile) 
        {
            new_block->tr = new_right_space_tile;
            temp_tile = new_right_space_tile->rt;
            while (temp_tile && temp_tile->right_top.x >= new_block->right_top.x) 
            {
                if (temp_tile->left_bottom.x < new_block->right_top.x) 
                    new_block->rt = temp_tile;
                temp_tile = temp_tile->bl;
            }
        }
        // Update new_block's neighbor (bl, lb)
        if (center_spaceTile == bottom_space_tile) 
        {
            new_block->bl = center_spaceTile;
            temp_tile = center_spaceTile->lb;
            while (temp_tile && temp_tile->left_bottom.x <= new_block->left_bottom.x) 
            {
                if (temp_tile->right_top.x > new_block->left_bottom.x) 
                    new_block->lb = temp_tile;
                temp_tile = temp_tile->tr;
            }
        }
        center_spaceTile->right_top.x = new_block->left_bottom.x;
        center_spaceTile->tr = new_block;
        center_spaceTile->rt = new_top_spaceTile;
        new_top_spaceTile = center_spaceTile;
    }
}

void Layout::MergeRight(vector<Tile*> right_tiles)
{
    Tile* first_tile = *(right_tiles.begin());
    Tile* last_tile = right_tiles.back();

    if (first_tile->rt && MayMergeable(first_tile, first_tile->rt))
        right_tiles.insert(right_tiles.begin(), first_tile->rt);

    if (last_tile->lb && MayMergeable(last_tile, last_tile->lb))
        right_tiles.push_back(last_tile->lb);

    Tile* temp_tile;
    for (int i = 0; i < right_tiles.size()-1; ++i)
     {
        Tile* curr = right_tiles[i];
        Tile* next = right_tiles[i+1];
        if (IsMergeable(curr, next)) 
        {
            SetPtr_Merge(curr, next, &Tile::rt, &Tile::lb, &Tile::bl);
            SetPtr_Merge(curr, next, &Tile::tr, &Tile::bl, &Tile::lb);
            SetPtr_Merge(curr, next, &Tile::bl, &Tile::tr, &Tile::rt);
            next->right_top = curr->right_top;
            next->rt = curr->rt;
            next->tr = curr->tr;
            TileDeletion(curr);
        }
    }
}
void Layout::MergeLeft(vector<Tile*> left_tiles)
{
    Tile* first_tile = *(left_tiles.begin());
    Tile* last_tile = left_tiles.back();

    if(first_tile->lb && MayMergeable(first_tile, first_tile->lb))
        left_tiles.insert(left_tiles.begin(), first_tile->lb);
    if(last_tile->rt && MayMergeable(last_tile, last_tile->rt))
        left_tiles.push_back(last_tile->rt);

    Tile* temp_tile;
    for (int i = 0; i < left_tiles.size()-1 ; i++)
    {
        Tile* curr = left_tiles[i];
        Tile* next = left_tiles[i+1];
        if(IsMergeable(curr, next))
        {
            SetPtr_Merge(curr, next, &Tile::lb, &Tile::rt, &Tile::tr);
            SetPtr_Merge(curr, next, &Tile::tr, &Tile::bl, &Tile::lb);
            SetPtr_Merge(curr, next, &Tile::bl, &Tile::tr, &Tile::rt);
            next->left_bottom = curr->left_bottom;
            next->lb = curr->lb;
            next->bl = curr->bl;
            TileDeletion(curr);
        }
    }
}

bool Layout::MayMergeable(Tile* t1,Tile* t2)
{
    return t1->left_bottom.x == t2->left_bottom.x && 
           t1->right_top.x == t2->right_top.x;
}
bool Layout::IsMergeable(Tile* t1,Tile* t2)
{
    return t1->left_bottom.x == t2->left_bottom.x && 
           t1->right_top.x == t2->right_top.x && 
           t1->type == space && t2->type == space;
}
void Layout::SetPtr_Merge(Tile* curr, Tile* next, Tile* (Tile::*dir1), Tile* (Tile::*dir2), Tile* (Tile::*dir3))
{
    Tile* temp_tile = curr->*dir1;
    while (temp_tile && temp_tile->*dir2 == curr)
    {
        temp_tile->*dir2 = next;
        temp_tile = temp_tile->*dir3;
    }
}
void Layout::SetPtr_Merge(Tile* curr, Tile* new_block, Tile* (Tile::*dir1), Tile* (Tile::*dir2), Tile* (Tile::*dir3), Tile* bottom_space_tile)
{
    Tile* temp_tile = curr->*dir1;
    while (temp_tile && temp_tile->*dir2 == curr)
    {
        temp_tile->*dir2 = (curr == bottom_space_tile) ? new_block : nullptr;
        temp_tile = temp_tile->*dir3;
    }
}
