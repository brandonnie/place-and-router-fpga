#include "graph_painter.h"
Block::Block(int _x1, int _y1, int _x2, int _y2, string _block_id)
{
    x1 = _x1;
    y1 = _y1;
    x2 = _x2;
    y2 = _y2;
    this->block_id = _block_id;
}

bool Block::is_inside_block(float x, float y){
    // cout << "Block Coordinate: x1: " << x1 << ", y1: " << y1 << ", x2: " << x2 << ", y2: " << y2 << endl;
    // cout << "Input coorinate: x: " << x << ", y:" << y <<endl;
    if (x > x1 && x < x2 && y > y1 && y < y2){
        //this->draw_block(RED, true);
        return true;
    }
    return false;
}

void Block::draw_block(int cindex, bool fill){
    if (fill){
        setcolor (cindex);
        fillrect(this->x1, this->y1, this->x2, this->y2);
    } else{
        setcolor (cindex);
        setlinestyle (SOLID);
        drawrect(this->x1, this->y1, this->x2, this->y2);
    }
    setfontsize (12);
    cindex++;
    setcolor(cindex);
    drawtext (x1 + (x2-x1)/2,y1 + (y2-y1)/2,this->block_id.c_str(),500.);
}

void Block::draw_track(int cindex, int track_id, float end_value){
    float start_x, start_y, end_x, end_y;
    if (track_id == 1){
        start_x = x1+(x2-x1)/2;
        start_y = y2;
        end_x = start_x;
        end_y = end_value;
    } else if (track_id == 2){
        start_x = x2;
        start_y = y1+(y2-y1)/2;
        end_x = end_value;
        end_y = start_y;
    }else if (track_id == 3){
        start_x = x1+(x2-x1)/2;
        start_y = y1;
        end_x = start_x;
        end_y = end_value;
    }else {
        start_x = x1;
        start_y = y1+(y2-y1)/2;
        end_x = end_value;
        end_y = start_y; 
    }
    setcolor (cindex);
    setlinestyle (SOLID);
    drawline(start_x, start_y, end_x, end_y);
    fillarc (end_x,end_y,(x2-x1)/15,0.,360.);
}

bool Block::operator == (const Block &Ref) const {
    if (this->x1 == Ref.x1 && this->x2 == Ref.x2 && this->y1 == Ref.y1 && this->y2 == Ref.y2){
        return true;
    }
    return false;
}

Graph_Painter::Graph_Painter(int _dimension, int _tracks_per_channel, int _num_logic_blocks){
    this->dimension = _dimension;
    this->tracks_per_channel = _tracks_per_channel;
    this->num_logic_blocks = _num_logic_blocks;
    this->network_ptr = new Network(_dimension,_tracks_per_channel,_num_logic_blocks);
    this->hovering_block = NULL;
    length_for_each_blob = 1000/this->dimension;
    space_for_channel = length_for_each_blob/4;
    space_per_track = space_for_channel/this->tracks_per_channel;
    length_for_block = (length_for_each_blob - 2*space_for_channel)/this->num_logic_blocks;
    this->create_block_matrix();
    for (int x = 0; x <= this->dimension*2; x++){
        int y_top = this->dimension-1;
        if (x%2){
            y_top = this->dimension;
        }
        for (int y = 0; y <= y_top; y++){
            for (int z = 0; z < this->tracks_per_channel; z++){
                vector<float> curr_line_coordinates = this->convert_track_to_coordinates(&this->network_ptr->get_channel(x, y), z);
            }
        }
    }
}

void Graph_Painter::create_block_matrix(){
    for (int x = 0; x < this->dimension; x++){
        this->block_matrix.push_back(vector<vector<Block> >());
        for (int y = 0; y < this->dimension; y++){
            this->block_matrix[x].push_back(vector<Block>());
            for (int z = 0; z < this->num_logic_blocks; z++){
                vector<float> curr_block_coordinates = this->compute_block_coord(x, y, z);
                char block_char = 'a' + z;
                string block_id = "(" + to_string(x) + ", " + to_string(y) + ", " + block_char + ")";
                this->block_matrix[x][y].push_back(Block(curr_block_coordinates[0], curr_block_coordinates[1], curr_block_coordinates[2], curr_block_coordinates[3], block_id));
            }
        }
    }
}

vector<float> Graph_Painter::compute_block_coord(int x, int y, int z){
    vector<float> coordinates;
    float x1 = x * length_for_each_blob + space_for_channel + z*length_for_block;
    float x2 = x1 + length_for_block;
    float y1 = y * length_for_each_blob + space_for_channel + z*length_for_block;
    float y2 = y1 + length_for_block;
    coordinates.push_back(x1);
    coordinates.push_back(y1);
    coordinates.push_back(x2);
    coordinates.push_back(y2);
    return coordinates;
}

vector<float> Graph_Painter::convert_track_to_coordinates(Channel* channel_ptr, int track_id){
    vector<float> coordinates;
    int x = channel_ptr->get_x();
    int y = channel_ptr->get_y();
    float space_per_line = (space_for_channel*2)/(this->tracks_per_channel + 1);
    if (channel_ptr->is_vertical() ){
        float x_start = (x/2) * length_for_each_blob - space_for_channel + space_per_line;
        float x1 = x_start + track_id*space_per_line;
        float x2 = x1;
        float y1 = space_for_channel + y*length_for_each_blob;
        float y2 = y1 + length_for_each_blob/2;
        coordinates.push_back(x1);
        coordinates.push_back(y1);
        coordinates.push_back(x2);
        coordinates.push_back(y2);
        channel_ptr->track_coordinates.push_back(coordinates);
    } else {
        float y_start = y * length_for_each_blob + space_for_channel - space_per_line;
        float y1 = y_start - track_id*space_per_line;
        float y2 = y1;
        float x1 = space_for_channel + ((x-1)/2) * length_for_each_blob;
        float x2 = x1 + length_for_each_blob/2;
        coordinates.push_back(x1);
        coordinates.push_back(y1);
        coordinates.push_back(x2);
        coordinates.push_back(y2);
        channel_ptr->track_coordinates.push_back(coordinates);
    }
    return coordinates;
}

vector<int> Graph_Painter::convert_block_to_channel_id(int block_x, int block_y, int block_z, int block_track){
    //result contains: x, y, block_z
    vector<int> results;
    if (block_track == 1){
        results.push_back(1+2*block_x);
        results.push_back(block_y+1);
    } else if (block_track == 2){
        results.push_back(2+2*block_x);
        results.push_back(block_y);
    } else if (block_track == 3){
        results.push_back(1+2*block_x);
        results.push_back(block_y);
    } else if (block_track == 4){
        results.push_back(2*block_x);
        results.push_back(block_y);
    }
    results.push_back(block_z);
    return results;
}

bool Graph_Painter::compute_path_using_block_id(int start_x, int start_y, int start_z, int start_track, int end_x, int end_y, int end_z, int end_track){
    vector<int> start_channel_id = this->convert_block_to_channel_id(start_x, start_y, start_z, start_track);
    vector<int> end_channel_id = this->convert_block_to_channel_id(end_x, end_y, end_z, end_track);
    bool computation_successful = this->network_ptr->compute_path(pair<int,int>(start_channel_id[0], start_channel_id[1]), pair<int,int>(end_channel_id[0], end_channel_id[1]), start_channel_id[2], end_channel_id[2]);
    if (computation_successful){
        vector<int> start_info;
        start_info.push_back(start_x);
        start_info.push_back(start_y);
        start_info.push_back(start_z);
        start_info.push_back(start_track);
        vector<int> end_info;
        end_info.push_back(end_x);
        end_info.push_back(end_y);
        end_info.push_back(end_z);
        end_info.push_back(end_track);
        pair<vector<int>,vector<int> > info_pair = make_pair(start_info, end_info);
        this->path_endpoint_information.push_back(info_pair);
    }
    return computation_successful;
}

void Graph_Painter::update_highlight_list(float x, float y){
    // determines if the clicked point is within a path endpoint
    vector<int> new_paths_to_highlight;
    vector<Block> new_blocks_to_highlight;
    for (unsigned int i = 0; i < this->path_endpoint_information.size(); i++){
        vector<int> start_info = this->path_endpoint_information[i].first;
        vector<int> end_info = this->path_endpoint_information[i].second;
        if (this->block_matrix[start_info[0]][start_info[1]][start_info[2]].is_inside_block(x, y)){
            new_paths_to_highlight.push_back(i);
            new_blocks_to_highlight.push_back(block_matrix[start_info[0]][start_info[1]][start_info[2]]);
        } else if (this->block_matrix[end_info[0]][end_info[1]][end_info[2]].is_inside_block(x, y)){
            new_paths_to_highlight.push_back(i);
            new_blocks_to_highlight.push_back(block_matrix[end_info[0]][end_info[1]][end_info[2]]);
        }
    }
    if (new_paths_to_highlight.size()){
        this->path_to_highlight.clear();
        this->path_to_highlight = new_paths_to_highlight;
        this->blocks_to_highlight.clear();
        this->blocks_to_highlight = new_blocks_to_highlight;
    }
}

bool Graph_Painter::is_highlight_block (const Block &Ref){
    for (Block highlight_block: this->blocks_to_highlight){
        if (Ref == highlight_block){
            return true;
        }
    }
    return false;
}

void Graph_Painter::paint_tracks(int line_width, int cindex, int style){
    for (int x = 0; x <= this->dimension*2; x++){
        int y_top = this->dimension-1;
        if (x%2){
            y_top = this->dimension;
        }
        for (int y = 0; y <= y_top; y++){
            for (int z = 0; z < this->tracks_per_channel; z++){
                this->network_ptr->get_channel(x, y).draw_channel(line_width, cindex, style);
            }
        }
    }
}

void Graph_Painter::paint_blocks(int cindex, bool fill){
    for (int x = 0; x < this->dimension; x++){
        for (int y = 0; y < this->dimension; y++){
            for (int z = 0; z < this->num_logic_blocks; z++){
                this->block_matrix[x][y][z].draw_block(cindex, fill);
            }
        }
    }
}

void Graph_Painter::paint_paths(int line_width, int cindex, int style){
    if (cindex < BLUE){
        cindex = BLUE;
    }
    setlinewidth (line_width);
    setcolor (cindex);
    setlinestyle (style);
    int path_color = cindex;
    vector< vector<pair<Channel*, int> > > paths_to_draw = this->network_ptr->get_paths_to_draw();
    for (unsigned int i = 0; i < paths_to_draw.size(); i++){
        auto path = paths_to_draw[i];
        setcolor(path_color);
        path_color++;
        if (path_color > MAGENTA){
            path_color = BLUE;
        }
        if (path_color == RED){
            // red is reserved for highlight only
            path_color++;
        }
        this->paint_single_path(i, path_color);
    }
}

void Graph_Painter::paint_single_path(int path_num, int cindex){
    vector<pair<Channel*, int> > path = this->network_ptr->get_paths_to_draw()[path_num];
    vector<float> prev_line_coordinates;
    for (unsigned int j = 0; j < path.size(); j++){
        pair<Channel*, int> endpoint = path[j];
        vector<float> curr_line_coordinates = this->convert_track_to_coordinates(endpoint.first, endpoint.second);
        if (j == 0 || j == (path.size() - 1)){
            // draw the connection between track and logic block for first and last segment
            vector<int> endpoint_info;
            if (j){
                endpoint_info = this->path_endpoint_information[path_num].second;
            } else {
                endpoint_info = this->path_endpoint_information[path_num].first;
                // check that the start point of this path is equivalent to the start block
                // this will be false if the start point is from part of the equivalent tracks
                vector<int> converted_channel_id = this->convert_block_to_channel_id(endpoint_info[0], endpoint_info[1], endpoint_info[2], endpoint_info[3]);
                if (endpoint.first->get_x() != converted_channel_id[0] || endpoint.first->get_y() != converted_channel_id[1]){
                    prev_line_coordinates = curr_line_coordinates;
                    continue;
                }
            }
            float end_coordinate;
            if (endpoint.first->is_vertical()){
                end_coordinate = curr_line_coordinates[0];
            } else {
                end_coordinate = curr_line_coordinates[1];
            }
            this->block_matrix[endpoint_info[0]][endpoint_info[1]][endpoint_info[2]].draw_track(cindex, endpoint_info[3], end_coordinate);
        }
        drawline (curr_line_coordinates[0],curr_line_coordinates[1],curr_line_coordinates[2],curr_line_coordinates[3]);
        if (prev_line_coordinates.size()){
            vector<float> connection_line = this->network_ptr->find_shortest_connection(curr_line_coordinates, prev_line_coordinates);
            drawline (connection_line[0], connection_line[1], connection_line[2], connection_line[3]);
        }
        prev_line_coordinates = curr_line_coordinates;
    }
}

void Graph_Painter::highlight_path(){
    for (int path_num: this->path_to_highlight){
        setlinewidth (8);
        setcolor (RED);
        setlinestyle (SOLID);
        this->paint_single_path(path_num, RED);
    }
    for (Block block_to_highlight: this->blocks_to_highlight){
        block_to_highlight.draw_block(RED, true);
    }
}