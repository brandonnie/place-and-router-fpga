#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <chrono>
#include <thread>
#include "graphics.h"
#include "Netlist.h"
using namespace std;

// Callbacks for event-driven window handling.
void delay (void);
void drawscreen_new (void);
void act_on_change_mode_func (void (*drawscreen_ptr) (void));
void act_on_button_press (float x, float y);
void act_on_mouse_move (float x, float y);
void act_on_key_press (char c);
void draw_failure_message();

void parse_file (string file_name);

Netlist* my_netlist;
bool use_bfs = true;
int init_best_soln = 0;

int main (int argc, char *argv[]){
    string test_case = "tests/cct1";
    if (argc > 1){
        cout << "You entered testcase: ";
        test_case = argv[1];
        cout << test_case << endl;
    } else {
        cout << "No test case specified, default test: tests/cct1\n";
    }
    my_netlist = new Netlist();
    parse_file(test_case);
    //my_netlist->lowest_bound_first_search();
    my_netlist->BFS();
    init_graphics("Partitioner", WHITE);
    init_world (-1.,-1.,100.,100.);
    create_button ("Window", "LBF Mode", act_on_change_mode_func);

    event_loop(act_on_button_press, NULL , NULL, drawscreen_new);
    return 0;
}

void act_on_button_press (float x, float y){
    drawscreen_new();
}

void drawscreen_new (void) {
    set_draw_mode (DRAW_NORMAL);
    clearscreen();
    my_netlist->draw_nodes(my_netlist->decision_tree);
}

void act_on_change_mode_func (void (*drawscreen_ptr) (void)){
    my_netlist->best_cut_size = init_best_soln;
    if (use_bfs){
        change_button_text("LBF Mode", "BFS Mode");
        use_bfs = false;
        my_netlist->lowest_bound_first_search();
    } else {
        change_button_text("BFS Mode", "LBF Mode");
        use_bfs = true;
        my_netlist->BFS();
    }
    drawscreen_new();
}

void parse_file (string file_name){
    std::ifstream infile(file_name);
    if (infile.fail()){
        cout << "Failed to open file " << file_name << endl;
        exit(EXIT_FAILURE);
    }
    string line;
    string s;
    while (getline(infile, line)){
        istringstream iss(line);
        vector <int> info_line;
        while (getline(iss, s, ' ')){
            int num_to_push = stoi(s);
            info_line.push_back(num_to_push);
        }
        if (info_line[0] == -1){
            continue;
        }
        my_netlist->add_cell(info_line);
    }
    my_netlist->compute_fanout();
    my_netlist->compute_partition_size(true);
    for (unsigned int i = 0; i < 500; i++){
        my_netlist->compute_init_soln();
    }
    init_best_soln = my_netlist->best_cut_size;
    //my_netlist->compute_init_soln_greedy();
    cout << "init solution computed\n";
    infile.close();
}