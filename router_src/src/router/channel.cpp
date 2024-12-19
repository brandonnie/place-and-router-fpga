#include "channel.h"

using namespace std;

Channel::Channel(int _x, int _y, int _width, int _num_logic_blocks):
    x(_x),
    y(_y)
{
    this -> id.first = _x;
    this -> id.second = _y;
    this -> width = _width;
    this -> num_logic_blocks = _num_logic_blocks;
    for (int i = 0; i < _width; i++){
        this->track_available.push_back(true);
        this->track_distance.push_back(-1);
        this->track_taken.push_back(false);
        this->track_connected_logic_block.push_back(-1);
        this->track_equivalent_channel.push_back(pair<int,int>(-1,-1));
        this->track_equivalent_track.push_back(-1);
    }
}

pair<int, int> Channel::get_id(){
    return id;
}

int Channel::get_width(){
    return this->width;
}

const int Channel::get_x() const{
    return this->x;
}

const int Channel::get_y() const{
    return this->y;
}

void Channel::add_neighbour (pair <int, int> neighbour, double weight){
    this->adjacent.push_back(neighbour);
    this->adjacent_weight.push_back(weight);
}

void Channel::reset_node(){
    for (int i = 0; i < this->width; i++){
        this->track_distance[i] = -1;
        if (!this->track_taken[i]){
            this->track_available[i] = true;
        }
    }
}

vector<pair <int,int> > Channel::get_neighbours(){
    return (this->adjacent);
}

bool Channel::get_track_available(int track_id){
    if (track_id > this->track_available.size()){
        cout << "Invalid track ID in get_track_available" << endl;
        cout << "Channel ID: x: " << this->x << ", y: " << this->y << endl;
        cout << "Track ID: " << track_id << endl;
        return false;
    }
    return this->track_available[track_id];
}

void Channel::set_track_available(int track_id, bool new_value){
    if (track_id > this->track_available.size()){
        cout << "Invalid track ID in get_track_available" << endl;
        cout << "Channel ID: x: " << this->x << ", y: " << this->y << endl;
        cout << "Track ID: " << track_id << endl;
        return;
    }
    this->track_available[track_id] = new_value;
}

int Channel::get_track_distance(int track_id){
    if (track_id > this->track_available.size()){
        cout << "Invalid track ID in get_track_distance" << endl;
        cout << "Channel ID: x: " << this->x << ", y: " << this->y << endl;
        cout << "Track ID: " << track_id << endl;
        return -2;
    }
    return this->track_distance[track_id];
}

void Channel::set_track_distance(int track_id, int new_value){
    if (track_id > this->track_available.size()){
        cout << "Invalid track ID in get_track_distance" << endl;
        cout << "Channel ID: x: " << this->x << ", y: " << this->y << endl;
        cout << "Track ID: " << track_id << endl;
        return;
    }
    this->track_distance[track_id] = new_value;
}

bool Channel::get_track_taken(int track_id){
    if (track_id > this->track_available.size()){
        cout << "Invalid track ID in get_track_taken" << endl;
        cout << "Channel ID: x: " << this->x << ", y: " << this->y << endl;
        cout << "Track ID: " << track_id << endl;
        return true;
    }
    return this->track_taken[track_id];
}

void Channel::set_track_taken(int track_id, bool new_value){
    if (track_id > this->track_available.size()){
        cout << "Invalid track ID in get_track_taken" << endl;
        cout << "Channel ID: x: " << this->x << ", y: " << this->y << endl;
        cout << "Track ID: " << track_id << endl;
        return;
    }
    this->track_taken[track_id] = new_value;
}

int Channel::get_track_connected_logic_block(int track_id){
    if (track_id > this->track_connected_logic_block.size()){
        cout << "Invalid track ID in get_track_taken" << endl;
        cout << "Channel ID: x: " << this->x << ", y: " << this->y << endl;
        cout << "Track ID: " << track_id << endl;
        return -2;
    }
    return this->track_connected_logic_block[track_id];
}
void Channel::set_track_connected_logic_block(int track_id, int new_value){
    if (track_id > this->track_connected_logic_block.size()){
        cout << "Invalid track ID" << endl;
        cout << "Channel ID: x: " << this->x << ", y: " << this->y << endl;
        cout << "Track ID: " << track_id << endl;
        return;
    } else if (new_value > this->num_logic_blocks-1){
        cout << "Invalid logic block ID" << endl;
        cout << "Channel ID: x: " << this->x << ", y: " << this->y << endl;
        cout << "New value: " << track_id << endl;
        return;
    }
    this->track_connected_logic_block[track_id] = new_value;
}

void Channel::set_equivalent_track(int track_id, pair<int,int> equiv_channel, int equiv_track){
    this->track_equivalent_channel[track_id] = equiv_channel;
    this->track_equivalent_track[track_id] = equiv_track;
}

bool Channel::is_equivalent_track(int track_id, pair<int,int> equiv_channel, int equiv_track){
    if (this->track_equivalent_channel[track_id] == equiv_channel && this->track_equivalent_track[track_id] == equiv_track){
        return true;
    }
    return false;
}

bool Channel::is_vertical(){
    return !(this->x % 2);
}

void Channel::draw_channel(int line_width, int cindex, int style){
    setlinewidth (line_width);
    setcolor (cindex);
    setlinestyle (style);
    for (vector<float> track_coor: this->track_coordinates){
        drawline(track_coor[0], track_coor[1], track_coor[2], track_coor[3]);
    }
}

ostream& operator<<(ostream& os, const Channel& ch)
{
    os << "Channel ID: (" << ch.x << ", " << ch.y << ")\n";
    return os;
}