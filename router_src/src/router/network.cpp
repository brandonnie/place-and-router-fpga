#include "network.h"

Network::Network(int _dimension, int _tracks_per_channel, int _num_logic_blocks)
{
    this->dimension = _dimension;
    this->tracks_per_channel = _tracks_per_channel;
    this->num_logic_blocks = _num_logic_blocks;
    this->max_x = _dimension*2;
    this->max_y = _dimension;
    this->debug = false;
    total_distance = 0;
    optimiza_for_resource = false;
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

vector<pair<int,int> > Network::find_equivalent_channels(pair<int,int> start, int track_id){
    vector <pair<int,int> > equivalent_channels;
    for (vector<pair<Channel*, int> > path: this->paths_to_draw){
        if (path[0].first->get_id() == start && path[0].second == track_id){
            // we found equivalent start point, now need to set all of this pre-computed path as distance 0
            for (pair<Channel*, int> segment: path){
                equivalent_channels.push_back(segment.first->get_id());
            }
        }
    }
    return equivalent_channels;
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
    bool route_found = false;
    for (int current_track = 0; current_track < this->tracks_per_channel; current_track++){
        if (debug) cout << "Current track: " << current_track << endl;
        if (this->get_channel(start).is_equivalent_track(current_track, start, current_track)
            && this->get_channel(start).get_track_connected_logic_block(current_track) == start_logic_block
            && this->get_channel(end).is_equivalent_track(current_track, start, current_track)
            && this->get_channel(end).get_track_connected_logic_block(current_track) == end_logic_block){
            if (debug) cout << "Found exact same start and end point!\n";
            return false;
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
        queue<pair<int,int> > expansion_list;
        vector<pair<int,int> > equivalent_starts;
        if (this->optimiza_for_resource){
            equivalent_starts = this->find_equivalent_channels(start, current_track);
        }
        equivalent_starts.push_back(start);
        for (pair<int,int> equiv_channel: equivalent_starts){
            expansion_list.push(equiv_channel);
            this->get_channel(equiv_channel).set_track_available(current_track, false);
            this->get_channel(equiv_channel).set_track_distance(current_track, 0);
        }
        while (!expansion_list.empty()){
            Channel &current_channel = this->get_channel(expansion_list.front());
            expansion_list.pop();
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
                if (neighbour.get_track_distance(current_track) != -1 || (!neighbour.get_track_available(current_track) && !neighbour.is_equivalent_track(current_track, start, current_track))){
                    continue;
                }
                if (debug) cout << "will push\n";
                neighbour.set_track_distance(current_track, current_channel.get_track_distance(current_track) + 1);
                expansion_list.push(n);
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
    this->total_distance = this->total_distance + distance + 1;
    vector<pair<Channel*, int> > current_path;
    while (distance >= 0 ){
        distance --;
        Channel& curr_channel = this->get_channel(curr_id);
        curr_channel.set_track_taken(track_id, true);
        current_path.insert(current_path.begin(), pair<Channel*,int>(&curr_channel, track_id));
        curr_channel.set_equivalent_track(track_id, start, track_id);
        vector<pair<int,int> > equivalent_neighbours;
        vector<pair<int,int> > non_equiv_neighbours;
        for (pair<int, int> n: curr_channel.get_neighbours()){
            Channel &neighbour = this->get_channel(n);
            if (neighbour.get_track_distance(track_id) == distance ){
                if (neighbour.is_equivalent_track(track_id, start, track_id)){
                    equivalent_neighbours.push_back(n);
                } else {
                    non_equiv_neighbours.push_back(n);
                }
            }
        }
        if (equivalent_neighbours.size()){
            curr_id = equivalent_neighbours[0];
        } else if (non_equiv_neighbours.size()) {
            curr_id = non_equiv_neighbours[0];
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
    cout << "####################\n\n\n";
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
    return shortest_line;
}