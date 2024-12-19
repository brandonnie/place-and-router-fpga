#ifndef CHANNEL_H
#define CHANNEL_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include "graphics.h"

using namespace std;

class Channel
{
private:
    pair <int, int> id;
    int x;
    int y;
    int width;
    int num_logic_blocks;
    vector<pair<int,int> > adjacent;
    vector<double> adjacent_weight;
    vector<bool> track_available;
    // the track distance represents how many steps it took to get here, -1 means it has not been traveled yet, -2 means something is wrong
    vector<int> track_distance;
    vector<bool> track_taken;
    vector<int> track_connected_logic_block;
    vector<pair<int,int> > track_equivalent_channel;
    vector<int> track_equivalent_track;
public:
    vector<vector<float> > track_coordinates;
    float x1;
    float x2;
    float y1;
    float y2;
    Channel(int _x, int _y, int _width, int _num_logic_blocks);

    pair<int, int> get_id();
    const int get_x() const;
    const int get_y() const;
    int get_width();

    //takes in the id of the neighbour, and also the weight (overhead to connect to this neighbour)
    void add_neighbour (pair <int, int> neighbour, double weight = 0);

    // reset the node to get ready for next routing, resets track_available and track_distance, unless a specific track is taken
    void reset_node();

    vector<pair <int,int> > get_neighbours();

    bool get_track_available(int track_id);
    void set_track_available(int track_id, bool new_value);

    // return the distance it took to get to this track, if the track_id doesn't exist, return -2
    int get_track_distance(int track_id);
    void set_track_distance(int track_id, int new_value);

    bool get_track_taken(int track_id);
    void set_track_taken(int track_id, bool new_value);

    int get_track_connected_logic_block(int track_id);
    void set_track_connected_logic_block(int track_id, int new_value);

    void set_equivalent_track(int track_id, pair<int,int> equiv_channel, int equiv_track);
    bool is_equivalent_track(int track_id, pair<int,int> equiv_channel, int equiv_track);

    bool is_vertical();

    void draw_channel(int line_width, int cindex, int style);

    friend ostream& operator<<(ostream& os, const Channel& ch);

};

#endif //CHANNEL_H