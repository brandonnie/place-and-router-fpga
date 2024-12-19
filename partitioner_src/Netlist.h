#ifndef SOLVER_H
#define SOLVER_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>
#include <queue>
#include <limits>
#include <cmath>
#include <map>
#include <tuple>
#include <algorithm>
#include "graphics.h"
using namespace std;

class Cell
{
public:
    int ID;
    bool fixed;
    bool highlighted;
    bool hovered;
    bool is_anchor;
    double solved_x, solved_y, curr_x, curr_y;
    int fanout;
    // a vector of net IDs connected to this cell.
    // The actual nets (Clique objects) can be found in Solver Object
    vector<int> connected_nets;
    vector<int> connected_cells;
    Cell(int _ID, bool _fixed, vector<int> _connected_nets);
    // converts the coordinates into draw coordinates, invert the y value
    // returns: x1, y1, x2, y2, x_middle, y_middle
    vector<double> get_draw_coordinates();
    double get_quadratic_distance(double new_x, double new_y);
    void draw_cell();
    bool is_inside(double x, double y);
};

class Clique
{
public:
    int ID;
    double weight;
    double HPWL;
    // contains all cells connected to this specific net
    vector<int> connected_cells;
    Clique(int _ID);
    double compute_weight(bool increase_weight = false, double increase_factor = 2.0);
};

class Decision_Node
{
public:
    bool is_root;
    bool visited;
    int lower_bound;
    int node_ID;
    int left_num, right_num;
    int level;
    double x, y;
    Decision_Node* left;
    Decision_Node* right;
    Decision_Node* parent;
    Decision_Node(bool _is_root, Decision_Node* _parent, int _node_ID, int _left_num, int _right_num, double _x = 50, double _y = 0);
    void draw_node(int colour, double size = 0.4);
};

struct compare
{
    bool operator()(const Cell* a, const Cell* b)
    {
        return a->fanout < b->fanout;
    }

    bool operator()(const Decision_Node* a, const Decision_Node* b)
    {
        return a->lower_bound > b->lower_bound;
    }
};

class Netlist
{
public:
    vector<Cell*> all_cells;
    vector<Cell*> sorted_cells;
    // contains all clique objects in the circuit, key: Clique ID, value: Clique pointer
    map<int, Clique*> all_nets;
    Decision_Node* decision_tree;
    Decision_Node* best_solution;
    int total_nodes_visited;
    int best_cut_size;
    int max_partition_size;
    Netlist(/* args */);
    vector<double> compute_child_xy(Decision_Node* parent, bool left_child);
    void add_cell(vector<int> input_line);
    vector<int> is_connected(int cell1, int cell2);
    void compute_fanout();
    void compute_partition_size(bool allow_imbalance);
    void compute_init_soln_greedy();
    void compute_init_soln();
    int compute_cut_size(Decision_Node* decision);
    int compute_cut_size(vector<vector<int> >& partition_nodes);
    vector<vector<int> > compute_partition(Decision_Node* decision);
    void BFS();
    void traverse_current_level(Decision_Node* leaf, int level);
    void lowest_bound_first_search();
    priority_queue <Decision_Node*,vector<Decision_Node*>, compare> find_leaf_nodes(Decision_Node* root_node);
    void DFS(Decision_Node* curr_node, priority_queue <Decision_Node*,vector<Decision_Node*>, compare>& Q);
    void remove_all_children(Decision_Node* root);
    int LB(Decision_Node* curr_node);
    void draw_nodes(Decision_Node* root);
};


#endif