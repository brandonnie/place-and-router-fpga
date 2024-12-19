#ifndef NETWORK_H
#define NETWORK_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <queue>
#include<limits>
#include <cmath>
#include "graphics.h"
#include "channel.h"

using namespace std;

class Network
{
private:
    vector<vector<Channel> > channel_network;
    int dimension;
    int tracks_per_channel;
    int num_logic_blocks;
    int max_x;
    int max_y;
    // a list of all paths to be drawn. The pair consists of (channel, track ID)
    vector< vector<pair<Channel*, int> > > paths_to_draw;
    // used in initialization, connects all tracks in network to their neighbours
    void connect_network();

public:
    int total_distance;
    bool optimiza_for_resource;
    bool debug;
    Network(int _dimension, int _tracks_per_channel, int _num_logic_blocks);
    ~Network();

    // returns a list of neighbours of a specific track
    vector<pair<int,int> > find_neighbours(int x, int y);

    // returns a reference to a channel
    Channel& get_channel(pair<int,int> coordinates);
    Channel& get_channel(int x, int y);

    vector< vector<pair<Channel*, int> > > get_paths_to_draw();

    vector<pair<int,int> > find_equivalent_channels(pair<int,int> start, int track_id);

    // this is the Maze algorithm that computes the path between 2 points
    // if a path can be found, it will store the path in paths_to_draw and return true
    // if a path cannot be found, it will return false
    bool compute_path(pair<int,int> start, pair<int,int> end, int start_logic_block, int end_logic_block);

    // back trace a found route, and mark the tracks along the path as unavailable
    // Add the path into paths_to_draw
    void occupy_path(pair<int,int> start, pair<int,int> end, int track_id, int start_logic_block, int end_logic_block);

    // Reset all channels to get ready for the next route
    void reset_network();

    // This function finds out the shortest line that is able to connect the 2 arguments, used for connecting paths
    vector<float> find_shortest_connection(vector<float> line1, vector<float> line2);
};

#endif // NETWORK_H

