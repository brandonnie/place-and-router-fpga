#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include "graphics.h"
#include "channel.h"
#include "network.h"
#include "graph_painter.h"

using namespace std;

// Callbacks for event-driven window handling.
void delay (void);
void drawscreen (void);
void drawscreen_new (void);
void act_on_new_button_func (void (*drawscreen_ptr) (void));
void act_on_clear_highlight_func (void (*drawscreen_ptr) (void));
void act_on_reduce_track_func (void (*drawscreen_ptr) (void));
void act_on_increase_track_func (void (*drawscreen_ptr) (void));
void act_on_change_optimization_func (void (*drawscreen_ptr) (void));
void act_on_button_press (float x, float y);
void act_on_mouse_move (float x, float y);
void act_on_key_press (char c);
void draw_help_message();
void draw_failure_message();
void recompute_routes();
void parse_file (string file_name);

Graph_Painter *my_graph_painter;
int dimension, tracks_per_channel;
bool dense_mode = false;
bool computation_successful = true;
bool optimize_for_tracks = false;
vector<vector<int> > navigation_info;
vector<string> failed_routes;


int main (int argc, char *argv[]){
    string test_case = "tests/cct1_dense";
    if (argc > 1){
        cout << "You entered testcase: ";
        test_case = argv[1];
        cout << test_case << endl;
    } else {
        cout << "No test case specified, default test: tests/cct1_dense\n";
    }
    parse_file(test_case);

    cout << "Total distance: " << my_graph_painter->network_ptr->total_distance << endl;

    init_graphics("Router", WHITE);

   /* still picture drawing allows user to zoom, etc. */
   // Set-up coordinates from (xl,ytop) = (0,0) to 
   // (xr,ybot) = (1000,1000)
   init_world (0.,0.,1000.,1000.);
   create_button ("Window", "Clear Highlight", act_on_clear_highlight_func);
   create_button ("Clear Highlight", "Reduce Track", act_on_reduce_track_func);
   create_button ("Reduce Track", "Increase Track", act_on_increase_track_func);
   create_button ("Increase Track", "Optimize Tracks", act_on_change_optimization_func);
   update_message("Click on any logic block to highlight its connections");

   set_mouse_move_input (true);
   event_loop(act_on_button_press, act_on_mouse_move, NULL, drawscreen_new);

   close_graphics ();
   printf ("Graphics closed down.\n");
}

void act_on_mouse_move (float x, float y) {
    // function to handle mouse move event, the current mouse position in the current world coordinate
    // as defined as MAX_X and MAX_Y in init_world is returned
    vector<vector<vector<Block> > >& my_block_matrix = my_graph_painter->block_matrix;
    for (unsigned int i = 0; i < my_block_matrix.size(); i++){
        for (unsigned int j = 0; j < my_block_matrix[i].size(); j++){
            for (unsigned int k = 0; k < my_block_matrix[i][j].size(); k++){
                if (my_block_matrix[i][j][k].is_inside_block(x, y)){
                    if (my_graph_painter->is_highlight_block(my_block_matrix[i][j][k])){
                        break;
                    }
                    // un-hover previous block
                    if (my_graph_painter->hovering_block){
                        if (!my_graph_painter->is_highlight_block(*my_graph_painter->hovering_block)){
                            my_graph_painter->hovering_block->draw_block(LIGHTGREY);
                        }
                    }
                    // update new hover block
                    my_graph_painter->hovering_block = &my_block_matrix[i][j][k];
                    // highlight new hover block
                    my_graph_painter->hovering_block->draw_block(BLUE);
                    break;
                }
            }
        }
    }
    //drawscreen_new();
}

void act_on_button_press (float x, float y){
    my_graph_painter->update_highlight_list(x, y);
    drawscreen_new();
}

void drawscreen_new (void) {
    set_draw_mode (DRAW_NORMAL);
    clearscreen();

    my_graph_painter->paint_blocks(LIGHTGREY);
    my_graph_painter->paint_tracks(3, LIGHTGREY, DASHED);
    my_graph_painter->paint_paths(3, BLUE, SOLID);
    my_graph_painter->highlight_path();

    draw_help_message();
    if (!computation_successful){
        draw_failure_message();
    }
}

void draw_help_message(){
    t_report* current_window_info = new t_report();
    report_structure(current_window_info);
    float text_x = ((current_window_info->xright - current_window_info->xleft) / 2) + current_window_info->xleft;
    float text_y = current_window_info->ytop + (20/current_window_info->ymult);
    float box_x1 = current_window_info->xleft;
    float box_x2 = current_window_info->xright;
    float box_y1 = current_window_info->ytop;
    float box_y2 = box_y1 + (40/current_window_info->ymult);
    setcolor (WHITE);
    fillrect (box_x1, box_y1, box_x2, box_y2);
    setfontsize (20);
    setcolor(RED);
    drawtext (text_x,text_y,"You can click on any logic block to highlight its connections!",numeric_limits<float>::infinity());
}

void draw_failure_message(){
    t_report* current_window_info = new t_report();
    report_structure(current_window_info);
    float text_x = ((current_window_info->xright - current_window_info->xleft) / 2) + current_window_info->xleft;
    float text_y = current_window_info->ytop + (current_window_info->ybot - current_window_info->ytop)/2;
    float box_x1 = current_window_info->xleft;
    float box_x2 = current_window_info->xright;
    float box_y1 = text_y - (20/current_window_info->ymult);
    float box_y2 = text_y + (20/current_window_info->ymult);
    setcolor (WHITE);
    fillrect (box_x1, box_y1, box_x2, box_y2);
    setfontsize (18);
    setcolor(RED);
    string fail_msg = "Current number of tracks: " + to_string(tracks_per_channel) + ". One or more routes failed to compute! Refer to command line output.";
    drawtext (text_x,text_y,fail_msg.c_str(),numeric_limits<float>::infinity());
    for (unsigned int i = 0; i < failed_routes.size(); i++){
        box_y1 += 40/current_window_info->ymult;
        box_y2 += 40/current_window_info->ymult;
        text_y += 40/current_window_info->ymult;
        setcolor (WHITE);
        fillrect (box_x1, box_y1, box_x2, box_y2);
        setcolor(RED);
        drawtext (text_x,text_y, failed_routes[i].c_str(),numeric_limits<float>::infinity());

    }
}

void act_on_clear_highlight_func (void (*drawscreen_ptr) (void)){
    my_graph_painter->path_to_highlight.clear();
    my_graph_painter->blocks_to_highlight.clear();
    drawscreen_new();
}

void act_on_reduce_track_func (void (*drawscreen_ptr) (void)){
    if (tracks_per_channel <= 0){
        return;
    }
    tracks_per_channel -= 1;
    recompute_routes();
    drawscreen_new();
}

void act_on_increase_track_func(void (*drawscreen_ptr) (void)){
    tracks_per_channel += 1;
    recompute_routes();
    drawscreen_new();
}

void act_on_change_optimization_func (void (*drawscreen_ptr) (void)){
    if (optimize_for_tracks){
        change_button_text("Optimize Delay", "Optimize Tracks");
        optimize_for_tracks = false;
    } else {
        change_button_text("Optimize Tracks", "Optimize Delay");
        optimize_for_tracks = true;
    }
    recompute_routes();
    drawscreen_new();
}

void recompute_routes(){
    free(my_graph_painter);
    computation_successful = true;
    failed_routes.clear();
    if (dense_mode){
        //dense mode
        dense_mode = true;
        my_graph_painter = new Graph_Painter(dimension, tracks_per_channel, 2);
    } else {
        my_graph_painter = new Graph_Painter(dimension, tracks_per_channel, 1);
    }
    if (optimize_for_tracks){
        my_graph_painter->network_ptr->optimiza_for_resource = true;
    }

    for (unsigned int i = 0; i < navigation_info.size(); i++){
        vector<int> nav_line = navigation_info[i];
        if (nav_line[0] == -1){
            break;
        }
        bool success;
        if (dense_mode)
            {success = my_graph_painter->compute_path_using_block_id(nav_line[0], nav_line[1],nav_line[2], nav_line[3], nav_line[4], nav_line[5], nav_line[6], nav_line[7]);}
        else
            {success = my_graph_painter->compute_path_using_block_id(nav_line[0], nav_line[1],0, nav_line[2], nav_line[3], nav_line[4], 0, nav_line[5]);}
        if (!success){
            computation_successful = false;
            if (dense_mode){
                failed_routes.push_back("Route failed from: (" + to_string(nav_line[0]) + ", " + to_string(nav_line[1]) + ") logic block " + string(1, 'a' + nav_line[2]) + " segment " + to_string(nav_line[3]) + " to (" + to_string(nav_line[4]) + ", " + to_string(nav_line[5]) + ") logic block " + string(1, 'a' + nav_line[2]) + " segment " + to_string(nav_line[7]));
            } else {
                failed_routes.push_back("Route failed from: (" + to_string(nav_line[0]) + ", " + to_string(nav_line[1]) + ") segment " + to_string(nav_line[2]) + " to (" + to_string(nav_line[3]) + ", " + to_string(nav_line[4]) + ") segment " + to_string(nav_line[5]));
            }
        }
    }
    if (!computation_successful){
        cout << "One or more path failed routing\n";
        for (unsigned int i = 0; i < failed_routes.size(); i++){
            cout << failed_routes[i] << endl;
        }
    }
    cout << "Total distance: " << my_graph_painter->network_ptr->total_distance << endl;
}

void parse_file (string file_name){
    std::ifstream infile(file_name);
    string line;
    int line_num = 0;
    string s;
    bool computation_successful = true;
    while (getline(infile, line)){
        istringstream iss(line);
        if (line_num == 0){
            iss >> dimension;
            cout << "dimension: " << dimension << endl;
        } else if (line_num == 1) {
            iss >> tracks_per_channel;
            cout << "tracks_per_channel: " << tracks_per_channel << endl;
        } else {
            vector<int> nav_line;
            while (getline(iss, s, ' ')){
                int num_to_push;
                if (s == "a"){
                    num_to_push = 0;
                } else if (s == "b"){
                    num_to_push = 1;
                } else{
                    num_to_push = stoi(s);
                }
                nav_line.push_back(num_to_push);
            }
            navigation_info.push_back(nav_line);
        }
        line_num++;
    }
    if (navigation_info[0].size() == 6){
        //non-dense mode
        my_graph_painter = new Graph_Painter(dimension, tracks_per_channel, 1);
    } else if (navigation_info[0].size() == 8){
        //dense mode
        dense_mode = true;
        my_graph_painter = new Graph_Painter(dimension, tracks_per_channel, 2);
    } else {
        cout << "Parse error\n";
        exit(EXIT_FAILURE);
    }
    for (unsigned int i = 0; i < navigation_info.size(); i++){
        vector<int> nav_line = navigation_info[i];
        if (nav_line[0] == -1){
            break;
        }
        bool success;
        if (dense_mode)
            {success = my_graph_painter->compute_path_using_block_id(nav_line[0], nav_line[1],nav_line[2], nav_line[3], nav_line[4], nav_line[5], nav_line[6], nav_line[7]);}
        else
            {success = my_graph_painter->compute_path_using_block_id(nav_line[0], nav_line[1],0, nav_line[2], nav_line[3], nav_line[4], 0, nav_line[5]);}
        if (!success){
            computation_successful = false;
            if (dense_mode){
                failed_routes.push_back("Route failed from: (" + to_string(nav_line[0]) + ", " + to_string(nav_line[1]) + ") logic block " + string(1, 'a' + nav_line[2]) + " segment " + to_string(nav_line[3]) + " to (" + to_string(nav_line[4]) + ", " + to_string(nav_line[5]) + ") logic block " + string(1, 'a' + nav_line[2]) + " segment " + to_string(nav_line[7]) + "\n");
            } else {
                failed_routes.push_back("Route failed from: (" + to_string(nav_line[0]) + ", " + to_string(nav_line[1]) + ") segment " + to_string(nav_line[2]) + " to (" + to_string(nav_line[3]) + ", " + to_string(nav_line[4]) + ") segment " + to_string(nav_line[5]) + "\n");
            }
        }
    }
    if (!computation_successful){
        cout << "One or more path failed routing\n";
        for (unsigned int i = 0; i < failed_routes.size(); i++){
            cout << failed_routes[i] << endl;
        }
    }
    infile.close();
}