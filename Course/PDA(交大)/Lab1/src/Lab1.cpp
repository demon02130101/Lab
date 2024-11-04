#include "tile.h"
#include "function.h"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        cout << "Please enter input and ouput files." << endl;
        return 1;
    }
    const string input_filename = argv[1];
    const string output_filename = argv[2];
    ifstream input_file;
    ofstream output_file;

    Layout layout;
    input_file.open(input_filename);
    if (input_file.fail())
    {
        cout << input_filename << " failed to execute!!!" << endl;
        return 1;
    }

    Tile *target_tile = nullptr;
    string line;
    int epoch = 2;

    getline(input_file, line);
    stringstream ss(line);
    ss >> layout.outline_width >> layout.outline_height;
    layout.Initial();
    while (getline(input_file, line))
    {
        stringstream ss(line);
        if (line[0] == 'P')
        {
            ss.ignore(1);
            Point target;
            ss >> target.x >> target.y;
            cout << "line: " << epoch++ << endl;
            target_tile = layout.PointFinding(target);
            layout.pointFind_list.push_back(target_tile->left_bottom);
        }
        else
        {
            int index, LB_x, LB_y, tile_width, tile_height;
            ss >> index >> LB_x >> LB_y >> tile_width >> tile_height;
            cout << "line: " << epoch++ << endl;
            Point LB = Point(LB_x, LB_y);
            Point RT = Point(LB_x + tile_width, LB_y + tile_height);
            layout.TileCreation(index, LB, RT);
        }
    }

    layout.SortBlock();
    output_file.open(output_filename);
    if (output_file.fail())
    {
        cout << output_filename << " fail to execute!!!" << endl;
        return 1;
    }
    output_file << layout.tile_count << endl;
    for (const auto& tile : layout.block_list)
    {//compute the amount of adjacent block and space tile of each solid tile
        const auto& neighbors = layout.GetNeighbors(tile);
        int block_count = 0, space_count = 0;
        for (const auto& neighborTile : neighbors)
        {
            if (neighborTile->type == solid)
            {
                block_count++;
            }
            else if (neighborTile->type == space)
            {
                space_count++;
            }
        }
        output_file << tile->index << " " << block_count << " " << space_count << endl;
    }
    for (const auto& point : layout.pointFind_list)
    {//print the result of point-finding
        output_file << point.x << " " << point.y << endl;
    }
    output_file.close();

    return 0;
}