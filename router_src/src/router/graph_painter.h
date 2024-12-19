#ifndef GRAPH_PAINTER_H
#define GRAPH_PAINTER_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <queue>
#include <limits>
#include "graphics.h"
#include "channel.h"
#include "network.h"

class Block
{
public:
    float x1;
    float y1;
    float x2;
    float y2;
    string block_id;
    Block(int _x1, int _y1, int _x2, int _y2, string _block_id);
    bool is_inside_block(float x, float y);
    void draw_block(int cindex, bool fill = true);
    void draw_track(int cindex, int track_id, float end_value);
    bool operator == (const Block &Ref) const;
};


class Graph_Painter
{
public:
    float length_for_each_blob;
    float space_for_channel;
    float space_per_track;
    float length_for_block;
    Network *network_ptr;
    int dimension;
    int tracks_per_channel;
    int num_logic_blocks;
    vector<int> path_to_highlight;
    vector<Block> blocks_to_highlight;
    Block *hovering_block;
    vector<vector<vector<Block> > > block_matrix;
    // vector contains block_x, block_y, block_z, block_track
    vector<pair<vector<int>,vector<int> > > path_endpoint_information;
    Graph_Painter(int _dimension, int _tracks_per_channel, int _num_logic_blocks);

    void create_block_matrix();

    vector<float> compute_block_coord(int x, int y, int z);

    vector<float> convert_track_to_coordinates(Channel* channel_ptr, int track_id);

    //result contains: x, y, block_z
    vector<int> convert_block_to_channel_id(int block_x, int block_y, int block_z, int block_track);

    void update_highlight_list(float x, float y);

    bool compute_path_using_block_id(int start_x, int start_y, int start_z, int start_track, int end_x, int end_y, int end_z, int end_track);

    bool is_highlight_block (const Block &Ref);

    void paint_tracks(int line_width, int cindex, int style);
    void paint_blocks(int cindex, bool fill = true);
    void paint_paths(int line_width, int cindex, int style);
    void paint_single_path(int path_num, int cindex);
    void highlight_path();
};



#endif //GRAPH_PAINTER_H