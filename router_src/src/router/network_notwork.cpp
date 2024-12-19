#include "network.h"

Network::Network(int _dimension, int _tracks_per_channel, int _num_logic_blocks)
{
    this->dimension = _dimension;
    this->tracks_per_channel = _tracks_per_channel;
    this->num_logic_blocks = _num_logic_blocks;
    this->max_x = _dimension*2;
    this->max_y = _dimension;
    this->debug = false;
    this->optimize_for_area = true;
    for (int x = 0; x <= this->max_x; x++){
        this->channel_network.push_back(vector<Channel>());
        int y_top = _dimension-1;
        if (x%2){
            y_top = _dimension;
        }
        for (int y = 0; y <= y_top; y++){
            channel_network[x].push_back(Channel(x, y, _tracks_per_channel, _num_logic_blocks));
        }
    }
    this->connect_network();
}

Network::~Network()
{
}

vector< vector<pair<Channel*, int> > > Network::get_paths_to_draw(){
    return this->paths_to_draw;
}

void Network::connect_network(){
    for (int x = 0; x < this->channel_network.size(); x++){
        for (int y = 0; y < this->channel_network[x].size(); y++){
            if (channel_network[x][y].get_x() != x || channel_network[x][y].get_y() !=y){
                cout << "ERROR: channel coordinate not equal to network coordinate" << endl;
                cout << "Channel coordinates: " << channel_network[x][y].get_x() << ", " << channel_network[x][y].get_y() << endl;
                cout << "Network coordinates: " << x << ", " << y << endl;
            }
            cout << "Network coordinates: " << x << ", " << y << endl;
            Channel* curr_channel_ptr = &(this->channel_network[x][y]);
            vector<pair<int,int> > neighbours = find_neighbours(x, y);
            for (pair<int,int> n: neighbours){
                curr_channel_ptr->add_neighbour(n);
            }
        }
    }
}

vector<pair<int,int> > Network::find_neighbours(int x, int y){
    vector<pair<int,int> > vertical_neighbours;
        vertical_neighbours.push_back(pair<int,int>(x-1, y+1));
        vertical_neighbours.push_back(pair<int,int>(x, y+1));
        vertical_neighbours.push_back(pair<int,int>(x+1, y+1));
        vertical_neighbours.push_back(pair<int,int>(x-1, y));
        vertical_neighbours.push_back(pair<int,int>(x, y-1));
        vertical_neighbours.push_back(pair<int,int>(x+1, y));
    vector<pair<int,int> > horizontal_neighbours;
        horizontal_neighbours.push_back(pair<int,int>(x-1, y));
        horizontal_neighbours.push_back(pair<int,int>(x+1, y));
        horizontal_neighbours.push_back(pair<int,int>(x-2, y));
        horizontal_neighbours.push_back(pair<int,int>(x+2, y));
        horizontal_neighbours.push_back(pair<int,int>(x-1, y-1));
        horizontal_neighbours.push_back(pair<int,int>(x+1, y-1));
    vector<pair<int,int> > actual_neighbours = channel_network[x][y].is_vertical()? vertical_neighbours: horizontal_neighbours;
    vector<pair<int,int> > results;
    for (auto neighbour: actual_neighbours){
        try{
            channel_network.at(neighbour.first).at(neighbour.second);
            results.push_back(neighbour);
        }
        catch (const std::out_of_range& e) {
            if (debug){
                cout << neighbour.first <<", " << neighbour.second << " Out of Range.\n";
            }
        }
    }
    if (debug){
        for (auto temp: results){
            cout << temp.first <<", " << temp.second << " is a neighbour.\n";
        }
    }
    return results;
}

Channel& Network::get_channel(pair<int,int> coordinates){
    return this->channel_network.at(coordinates.first).at(coordinates.second);
}
Channel& Network::get_channel(int x, int y){
    return this->channel_network.at(x).at(y);
}


bool Network::compute_path(pair<int,int> start, pair<int,int> end, int start_logic_block, int end_logic_block){
    try{
        channel_network.at(start.first).at(start.second);
        channel_network.at(end.first).at(end.second);
    }
    catch (const std::out_of_range& e) {
        cout << "Start Point (" << start.first << ", " << start.second << ") or End Point (" << end.first << ", " << end.second << ") Out Of Bound, the path is not computed!\n";
        return false;
    }
    cout << "Start routing from (" << start.first << ", " << start.second << ") logic block " << start_logic_block << " to (" << end.first << ", " << end.second << ") logic block " << end_logic_block << endl;
    int shortest_track_id = -1;
    int shortest_track_distance = numeric_limits<int>::max();
    int most_overlap_route = 0;
    int most_overlap_track_id = -1;
    bool route_found = false;
    for (int current_track = 0; current_track < this->tracks_per_channel; current_track++){
        if (debug) cout << "Current track: " << current_track << endl;
        if (this->get_channel(start).is_equivalent_track(current_track, start, current_track)
            && this->get_channel(start).get_track_connected_logic_block(current_track) == start_logic_block
            && this->get_channel(end).is_equivalent_track(current_track, start, current_track)
            && this->get_channel(end).get_track_connected_logic_block(current_track) == end_logic_block){
            if (debug) cout << "Found exact same start and end point!\n";
            return true;
        }
        if (!this->get_channel(start).get_track_available(current_track) && this->get_channel(start).is_equivalent_track(current_track, start, current_track)
            && this->get_channel(start).get_track_connected_logic_block(current_track) == start_logic_block){
            if (!this->get_channel(end).get_track_available(current_track)){
                if (debug) cout << "Equivalent start point, but end point not equivalent and not available, skipping.\n";
                continue;
            }
            // proceed as usual
            if (debug) cout << "Found equivalent start point!\n";
        }
        else if (!this->get_channel(start).get_track_available(current_track) || !this->get_channel(end).get_track_available(current_track)){
            if (debug) cout << "start or end unavailable, skipping this track\n";
            continue;
        }
        deque<pair<int,int> > expansion_list;
        expansion_list.push_back(start);
        this->get_channel(start).set_track_available(current_track, false);
        this->get_channel(start).set_track_distance(current_track, 0);
        while (!expansion_list.empty()){
            Channel &current_channel = this->get_channel(expansion_list.front());
            expansion_list.pop_front();
            if (debug) cout << "In loop Current channel: " << current_channel;
            current_channel.set_track_available(current_track, false);
            if (current_channel.get_id() == end){
                if (debug) cout << "Found destination\n";
                if (current_channel.get_track_distance(current_track) < shortest_track_distance){
                    if (debug) cout << "distance: " << current_channel.get_track_distance(current_track) << endl;
                    shortest_track_distance = current_channel.get_track_distance(current_track);
                    shortest_track_id = current_track;
                    route_found = true;
                }
                break;
            }
            for (pair<int, int> n: current_channel.get_neighbours()){
                Channel &neighbour = this->get_channel(n);
                if (debug) cout << "neighbour: " << n.first << ", " << n.second << endl;
                // distance not equal to -1 means we already traversed here
                // the second condition after the OR checks for the track is not available and not equivalent to start point.
                if (!neighbour.is_equivalent_track(current_track, start, current_track)){
                    if (neighbour.get_track_distance(current_track) != -1 || !neighbour.get_track_available(current_track)){
                        continue;
                    }
                } else{
                    // here the neighbour is equivalent to the start point, we always prefer
                    if (this->optimize_for_area){
                        if (debug) cout << "optimizing for area, will push\n";
                        neighbour.set_track_distance(current_track, current_channel.get_track_distance(current_track) + 1);
                        expansion_list.push_front(n);
                    }
                }
                if (debug) cout << "will push\n";
                neighbour.set_track_distance(current_track, current_channel.get_track_distance(current_track) + 1);
                expansion_list.push_back(n);
            }
        }
    }
    if (debug){
        cout<< "route found: " << route_found <<endl;
        cout << "shortest track: " << shortest_track_id << endl;
        cout << "shortest distance: " << shortest_track_distance << endl;
    }
    if (!route_found){
        return false;
    }
    this->occupy_path(start, end, shortest_track_id, start_logic_block, end_logic_block);
    this->reset_network();
    return true;
}

void Network::occupy_path(pair<int,int> start, pair<int,int> end, int track_id, int start_logic_block, int end_logic_block){
    this->get_channel(start).set_track_connected_logic_block(track_id, start_logic_block);
    this->get_channel(end).set_track_connected_logic_block(track_id, end_logic_block);
    pair<int,int> curr_id = end;
    int distance = this->get_channel(curr_id).get_track_distance(track_id);
    vector<pair<Channel*, int> > current_path;
    while (distance >= 0 ){
        distance --;
        Channel& curr_channel = this->get_channel(curr_id);
        curr_channel.set_track_taken(track_id, true);
        current_path.insert(current_path.begin(), pair<Channel*,int>(&curr_channel, track_id));
        curr_channel.set_equivalent_track(track_id, start, track_id);
        for (pair<int, int> n: curr_channel.get_neighbours()){
            Channel &neighbour = this->get_channel(n);
            if (neighbour.get_track_distance(track_id) == distance ){
                curr_id = n;
                break;
            }
        }
    }
    cout << "Path labeled\n";
    for (auto ch: current_path){
        cout<< *(ch.first) << "Track ID: " << track_id <<endl;
        if (ch.first->get_track_connected_logic_block(track_id) != -1){
            cout << "Connected to logic block: " << ch.first->get_track_connected_logic_block(track_id) << endl;
        }
    }
    this->paths_to_draw.push_back(current_path);
}

void Network::reset_network(){
    for (int x = 0; x < this->channel_network.size(); x++){
        for (int y = 0; y < this->channel_network[x].size(); y++){
            channel_network[x][y].reset_node();
        }
    }
}

vector<float> Network::find_shortest_connection(vector<float> line1, vector<float> line2){
    vector<float> shortest_line(4);
    float shortest_distance = numeric_limits<float>::infinity();
    float distance1 = sqrt(pow((line2[0] - line1[0]), 2) + pow((line2[1] - line1[1]), 2) );
    if (distance1 < shortest_distance){
        shortest_distance = distance1;
        shortest_line.clear();
        shortest_line.push_back(line1[0]);
        shortest_line.push_back(line1[1]);
        shortest_line.push_back(line2[0]);
        shortest_line.push_back(line2[1]);
    }
    float distance2 = sqrt(pow((line2[2] - line1[0]), 2) + pow((line2[3] - line1[1]), 2) );
    if (distance2 < shortest_distance){
        shortest_distance = distance2;
        shortest_line.clear();
        shortest_line.push_back(line1[0]);
        shortest_line.push_back(line1[1]);
        shortest_line.push_back(line2[2]);
        shortest_line.push_back(line2[3]);
    }
    float distance3 = sqrt(pow((line2[0] - line1[2]), 2) + pow((line2[1] - line1[3]), 2) );
    if (distance3 < shortest_distance){
        shortest_distance = distance3;
        shortest_line.clear();
        shortest_line.push_back(line1[2]);
        shortest_line.push_back(line1[3]);
        shortest_line.push_back(line2[0]);
        shortest_line.push_back(line2[1]);
    }
    float distance4 = sqrt(pow((line2[2] - line1[2]), 2) + pow((line2[3] - line1[3]), 2) );
    if (distance4 < shortest_distance){
        shortest_distance = distance4;
        shortest_line.clear();
        shortest_line.push_back(line1[2]);
        shortest_line.push_back(line1[3]);
        shortest_line.push_back(line2[2]);
        shortest_line.push_back(line2[3]);
    }
    // for (float coor : line1){
    //     cout << coor << ", ";
    // }
    // cout << endl;
    // for (float coor : line2){
    //     cout << coor << ", ";
    // }
    // cout << endl;
    // cout << "Distance 1: " << distance1 << endl;
    // cout << "Distance 2: " << distance2 << endl;
    // cout << "Distance 3: " << distance3 << endl;
    // cout << "Distance 4: " << distance4 << endl;
    // cout << "result\n";
    // for (float coor : shortest_line){
    //     cout << coor << ", ";
    // }
    // cout << endl;
    return shortest_line;
}