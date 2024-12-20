#include "Netlist.h"
using namespace std;

Cell::Cell(int _ID, bool _fixed, vector<int> _connected_nets)
{
    this->ID = _ID;
    this->fixed = _fixed;
    this->connected_nets = _connected_nets;
    this->highlighted = false;
    this->hovered = false;
    this->fanout = 0;
}

vector<double> Cell::get_draw_coordinates(){
    vector<double> coor;
    coor.push_back(curr_x-0.4);
    coor.push_back(25-curr_y-0.4);
    coor.push_back(curr_x+0.4);
    coor.push_back(25-curr_y+0.4);
    coor.push_back(curr_x);
    coor.push_back(25-curr_y);
    return coor;
}

void Cell::draw_cell(){
    // do not draw anchor cell
    if (this->is_anchor) return;
    setcolor (LIGHTGREY);
    if (this->fixed){
        setcolor (DARKGREEN);
    }
    if (this->hovered){
        setcolor(DARKGREY);
    }
    if (this->highlighted){
        setcolor(YELLOW);
    }
    setlinestyle(SOLID);
    setlinewidth(2);
    vector<double> draw_coordinates = this->get_draw_coordinates();
    fillrect (draw_coordinates[0],draw_coordinates[1],draw_coordinates[2],draw_coordinates[3]);
    setcolor(BLACK);
    drawrect(draw_coordinates[0],draw_coordinates[1],draw_coordinates[2],draw_coordinates[3]);
    setfontsize (12);
    string ID_str = to_string(this->ID);
    drawtext (draw_coordinates[4],draw_coordinates[5],ID_str.c_str(),2.5);
}

double Cell::get_quadratic_distance(double new_x, double new_y){
    return sqrt(pow((new_x - this->solved_x),2) + pow((new_y - this->solved_y), 2));
}

bool Cell::is_inside(double x, double y){
    vector<double> draw_coordinates = this->get_draw_coordinates();
    if (x > draw_coordinates[0] && x < draw_coordinates[2] && y > draw_coordinates[1] && y < draw_coordinates[3]){
        return true;
    }
    return false;
}

Clique::Clique(int _ID)
{
    this->ID = _ID;
}

double Clique::compute_weight(bool increase_weight, double increase_factor){
    double num_pins = this->connected_cells.size() * 1.0;
    double new_weight;
    if (increase_weight){
        new_weight = (2/num_pins) * increase_factor;
    } else {
        new_weight = (2/num_pins);
    }
    this->weight = new_weight;
    return new_weight;
}

Decision_Node::Decision_Node(bool _is_root, Decision_Node* _parent, int _node_ID, int _left_num, int _right_num, double _x, double _y)
{
    this->is_root = _is_root;
    this->visited = false;
    this->parent = _parent;
    this->node_ID = _node_ID;
    this->lower_bound = 0;
    this->left_num = _left_num;
    this->right_num = _right_num;
    this->level = _left_num + _right_num;
    this->left = NULL;
    this->right = NULL;
    this->x = _x;
    this->y = _y;
}

void Decision_Node::draw_node(int colour, double size){
    setlinestyle(SOLID);
    setlinewidth(2);
    double x1 = this->x - size;
    double y1 = this->y - size;
    double x2 = this->x + size;
    double y2 = this->y + size;
    if (this->parent){
        setcolor(BLACK);
        drawline(this->parent->x, this->parent->y, this->x, this->y);
    }
    setcolor (colour);
    fillrect(x1, y1, x2, y2);
    setcolor(BLACK);
    drawrect(x1, y1, x2, y2);
}

Netlist::Netlist(/* args */)
{
    // add a dummy element to all_cells to make cell ID and array ID align
    // since cell ID start counting from 1 and array ID start from 0
    this->all_cells.push_back(NULL);
    this->decision_tree = new Decision_Node(true, NULL, 0, 0, 0);
    this->decision_tree->x = 50;
    this->decision_tree->y = 0;
    this->max_partition_size = 0;
    this->best_cut_size = std::numeric_limits<int>::max();
}

vector<double> Netlist::compute_child_xy(Decision_Node* parent, bool left_child){
    vector<double> results;
    double delta_y = 100/this->all_cells.size();
    double delta_x = 25*pow(0.5,parent->level);
    if (left_child) delta_x = -delta_x;
    results.push_back(parent->x + delta_x);
    results.push_back(parent->y + delta_y);
    return results;
}

void Netlist::add_cell(vector<int> input_line){
    if (input_line.size() == 0){
        cout << "Empty line\n";
        return;
    }
    int cell_id = input_line[0];
    vector<int> conncted_nets(&input_line[1],&input_line[input_line.size()-1]);
    Cell* new_cell = new Cell(cell_id, false, conncted_nets);
    this->all_cells.push_back(new_cell);
    // create nets
    for (unsigned int i = 1; i < input_line.size() - 1; i++){
        int net_ID = input_line[i];
        Clique* net_of_interest;
        if (this->all_nets.find(net_ID) == this->all_nets.end()){
            Clique* new_clique = new Clique(net_ID);
            net_of_interest = new_clique;
            this->all_nets[net_ID] = new_clique;
        } else {
            net_of_interest = this->all_nets[net_ID];
        }
        net_of_interest->connected_cells.push_back(cell_id);
    }
}

vector<int> Netlist::is_connected(int cell1, int cell2){
    vector<int> cell1_nets = this->all_cells[cell1]->connected_nets;
    vector<int> cell2_nets = this->all_cells[cell2]->connected_nets;
    vector<int> all_connected_nets;
    for (int cell_1_net : cell1_nets){
        if (find(cell2_nets.begin(), cell2_nets.end(), cell_1_net)!=cell2_nets.end()){
            all_connected_nets.push_back(cell_1_net);
        }
    }
    return all_connected_nets;
}

void Netlist::compute_fanout(){
    for (unsigned int i = 1; i < this->all_cells.size(); i++){
        this->all_cells[i]->connected_cells.clear();
        for (unsigned int j = 1; j < this->all_cells.size(); j++){
            if (i==j) continue;
            if ((this->is_connected(i, j)).size()){
                this->all_cells[i]->connected_cells.push_back(j);
            }
        }
        this->all_cells[i]->fanout = this->all_cells[i]->connected_cells.size();
    }
}

void Netlist::compute_partition_size(bool allow_imbalance){
    if (allow_imbalance){
        // allow one partition to be 20% larger than the other partition
        // which means one partition can contain up to 60% of total nodes
        this->max_partition_size = (int)round((this->all_cells.size() - 1) * 0.6);
    } else {
        this->max_partition_size = (int)round((this->all_cells.size() - 1) * 0.5);
    }
}

void Netlist::compute_init_soln(){
    this->sorted_cells = this->all_cells;
    sort(this->sorted_cells.begin()+1, this->sorted_cells.end(), compare());
    Decision_Node* curr_node = this->decision_tree;
    // simple solution: put the first half of cells in one group and put the second half in another
    for (Cell* curr_cell : sorted_cells){
        if (!curr_cell) continue;
        int curr_ID = curr_cell->ID;
        int choose_left = rand() % 2;
        if (choose_left){
            if (curr_node->left_num + 1 <= this->max_partition_size){
                curr_node->left = new Decision_Node(false, curr_node, curr_ID, curr_node->left_num + 1, curr_node->right_num);
                curr_node = curr_node->left;
            } else {
                curr_node->right = new Decision_Node(false, curr_node, curr_ID, curr_node->left_num, curr_node->right_num + 1);
                curr_node = curr_node->right;
            }
        } else {
            if (curr_node->right_num + 1 <= this->max_partition_size){
                curr_node->right = new Decision_Node(false, curr_node, curr_ID, curr_node->left_num, curr_node->right_num + 1);
                curr_node = curr_node->right;
            } else {
                curr_node->left = new Decision_Node(false, curr_node, curr_ID, curr_node->left_num + 1, curr_node->right_num);
                curr_node = curr_node->left;
            }
        }
        curr_node->lower_bound = this->LB(curr_node);
    }
    int cut_size = this->compute_cut_size(curr_node);
    if (cut_size < this->best_cut_size){
        cout << "Initial solution updated, best cut size: " << cut_size << endl;
        this->best_cut_size = cut_size;
    }
    // remove the tree we already constructed
    this->remove_all_children(this->decision_tree);
}

int Netlist::compute_cut_size(Decision_Node* decision){
    vector<vector<int> > partitions = this->compute_partition(decision);
    return this->compute_cut_size(partitions);
}

int Netlist::compute_cut_size(vector<vector<int> >& partition_nodes){
    vector<int>& left_partition = partition_nodes[0];
    vector<int>& right_partition = partition_nodes[1];
    int cut_size = 0;
    if (!left_partition.size() || !right_partition.size()){
        return cut_size;
    }
    for (auto it : this->all_nets){
        Clique* curr_net = it.second;
        bool contains_left = false;
        bool contains_right = false;
        for (int cell : curr_net->connected_cells){
            if (find(left_partition.begin(), left_partition.end(), cell) != left_partition.end()){
                contains_left = true;
            }
            // else {
            //     contains_right = true;
            // }
            if (find(right_partition.begin(), right_partition.end(), cell) != right_partition.end()) {
                contains_right = true;
            }
        }
        if (contains_left && contains_right){
            cut_size++;
        }
    }
    return cut_size;
}

vector<vector<int> > Netlist::compute_partition(Decision_Node* decision){
    vector<int> left_partition, right_partition;
    vector<vector<int> > return_val;
    while (decision->parent){
        if (decision->left_num > decision->parent->left_num){
            left_partition.push_back(decision->node_ID);
        } else{
            right_partition.push_back(decision->node_ID);
        }
        decision = decision->parent;
    }
    return_val.push_back(left_partition);
    return_val.push_back(right_partition);
    return return_val;
}

void Netlist::BFS(){
    this->total_nodes_visited = 0;
    this->remove_all_children(this->decision_tree);
    this->best_solution = NULL;
    delete (this->decision_tree);
    this->decision_tree = new Decision_Node(true, NULL, 0, 0, 0);
    
    int tree_height = this->all_cells.size() - 1;
    for (int i = 1; i <= tree_height + 1; i++)
        this->traverse_current_level(this->decision_tree, i);
    cout << "Best cut: " << this->best_cut_size << ", total nodes traversed: " << this->total_nodes_visited << endl;
}

void Netlist::traverse_current_level(Decision_Node* leaf, int level){
    if (!leaf) return;
    if (level == 1){
        // if (leaf->visited) {
        //     return;
        // }
        this->total_nodes_visited++;
        if (leaf->level == this->all_cells.size() - 1){
            // terminal node
            int leaf_cost = this->compute_cut_size(leaf);
            if (leaf_cost < this->best_cut_size || (leaf_cost == this->best_cut_size && !this->best_solution)){
                cout << "Found new best cut: " << leaf_cost << endl;
                this->best_cut_size = leaf_cost;
                this->best_solution = leaf;
            }
            return;
        }
        if (leaf->lower_bound > this->best_cut_size){
            return;
        }
        if (leaf->left_num + 1 <= this->max_partition_size){
            vector<double> new_coord = this->compute_child_xy(leaf, true);
            leaf->left = new Decision_Node(false, leaf, this->sorted_cells[leaf->level + 1]->ID, leaf->left_num + 1, leaf->right_num, new_coord[0], new_coord[1]);
            leaf->left->lower_bound = this->LB(leaf->left);
        }
        if (leaf->right_num + 1 <= this->max_partition_size){
            vector<double> new_coord = this->compute_child_xy(leaf, false);
            leaf->right = new Decision_Node(false, leaf, this->sorted_cells[leaf->level + 1]->ID, leaf->left_num, leaf->right_num + 1, new_coord[0], new_coord[1]);
            leaf->right->lower_bound = this->LB(leaf->right);
        }
    } else if (level > 1){
        this->traverse_current_level(leaf->left, level - 1);
        this->traverse_current_level(leaf->right, level - 1);
    }
}

void Netlist::lowest_bound_first_search(){
    this->total_nodes_visited = 0;
    this->remove_all_children(this->decision_tree);
    this->best_solution = NULL;
    delete (this->decision_tree);
    this->decision_tree = new Decision_Node(true, NULL, 0, 0, 0);

    bool all_leaf_terminal = true;
    do{
        all_leaf_terminal = true;
        //compute all leafs
        priority_queue <Decision_Node*,vector<Decision_Node*>, compare> all_leaf_nodes;
        this->DFS(this->decision_tree, all_leaf_nodes);
        //priority_queue <Decision_Node*,vector<Decision_Node*>, compare> all_leaf_nodes = this->find_leaf_nodes(this->decision_tree);
        while (!all_leaf_nodes.empty()){
            Decision_Node* leaf = all_leaf_nodes.top();
            all_leaf_nodes.pop();
            leaf->visited = true;
            this->total_nodes_visited ++;
            if (leaf->level == this->all_cells.size() - 1){
                // terminal node
                int leaf_cost = this->compute_cut_size(leaf);
                if (leaf_cost <= this->best_cut_size){
                    cout << "Found new best cut: " << leaf_cost << endl;
                    this->best_cut_size = leaf_cost;
                    this->best_solution = leaf;
                }
                continue;
            }
            all_leaf_terminal = false;
            if (leaf->left_num + 1 <= this->max_partition_size){
                vector<double> new_coord = this->compute_child_xy(leaf, true);
                leaf->left = new Decision_Node(false, leaf, this->sorted_cells[leaf->level + 1]->ID, leaf->left_num + 1, leaf->right_num, new_coord[0], new_coord[1]);
                leaf->left->lower_bound = this->LB(leaf->left);
            }
            if (leaf->right_num + 1 <= this->max_partition_size){
                vector<double> new_coord = this->compute_child_xy(leaf, false);
                leaf->right = new Decision_Node(false, leaf, this->sorted_cells[leaf->level + 1]->ID, leaf->left_num, leaf->right_num + 1, new_coord[0], new_coord[1]);
                leaf->right->lower_bound = this->LB(leaf->right);
            }
        }
    }
    while (!all_leaf_terminal);
    cout << "Best cut: " << this->best_cut_size << ", total nodes traversed: " << this->total_nodes_visited << endl;
}

priority_queue <Decision_Node*,vector<Decision_Node*>, compare> Netlist::find_leaf_nodes(Decision_Node* root_node){
    priority_queue <Decision_Node*,vector<Decision_Node*>, compare> all_leaves;
    this->DFS(root_node, all_leaves);
    return all_leaves;
}

void Netlist::DFS(Decision_Node* curr_node, priority_queue <Decision_Node*,vector<Decision_Node*>, compare>& Q){
    if (!curr_node) return;

    if (!curr_node->left && !curr_node->right && !curr_node->visited && curr_node->lower_bound <= this->best_cut_size){
        Q.push(curr_node);
        return;
    }
    if (curr_node->left){
        this->DFS(curr_node->left, Q);
    }
    if (curr_node->right){
        this->DFS(curr_node->right, Q);
    }
}
int Netlist::LB(Decision_Node* curr_node){
    //return this->compute_cut_size(curr_node);
    vector<vector<int> > partitions = this->compute_partition(curr_node);
    if (!partitions[0].size() || !partitions[1].size()){
        return 0;
    }
    int lower_bound = this->compute_cut_size(partitions);
    int curr_level = curr_node->level;
    for (unsigned int i = curr_level + 1; i < this->sorted_cells.size(); i++){
        int new_node_ID = this->sorted_cells[i]->ID;
        // find out how many more cuts this new node will introduce with current partial partition
        // add the minimum impact to the lower bound
        int left_cut_size, right_cut_size;
        partitions[0].push_back(new_node_ID);
        left_cut_size = this->compute_cut_size(partitions);
        partitions[0].pop_back();
        partitions[1].push_back(new_node_ID);
        right_cut_size = this->compute_cut_size(partitions);
        partitions[1].pop_back();
        if (left_cut_size < right_cut_size){
            lower_bound += left_cut_size - lower_bound;
        } else {
            lower_bound += right_cut_size - lower_bound;
        }
    }
    return lower_bound;
}

// int Netlist::LB(Decision_Node* curr_node){
//     //return this->compute_cut_size(curr_node);
//     vector<vector<int> > partitions = this->compute_partition(curr_node);
//     if (!partitions[0].size() || !partitions[1].size()){
//         return 0;
//     }
//     int lower_bound = this->compute_cut_size(partitions);
//     int max_cut_increase = 0;
//     int curr_level = curr_node->level;
//     for (unsigned int i = curr_level + 1; i < this->sorted_cells.size(); i++){
//         int new_node_ID = this->sorted_cells[i]->ID;
//         // find out how many more cuts this new node will introduce with current partial partition
//         // add the minimum impact to the lower bound
//         int left_cut_size, right_cut_size;
//         partitions[0].push_back(new_node_ID);
//         left_cut_size = this->compute_cut_size(partitions);
//         partitions[0].pop_back();
//         partitions[1].push_back(new_node_ID);
//         right_cut_size = this->compute_cut_size(partitions);
//         partitions[1].pop_back();
//         int cut_increase = 0;
//         if (left_cut_size < right_cut_size){
//             cut_increase = left_cut_size - lower_bound;
//         } else {
//             cut_increase = right_cut_size - lower_bound;
//         }
//         if (cut_increase > max_cut_increase){
//             max_cut_increase = cut_increase;
//         }
//     }
//     lower_bound += max_cut_increase;
//     return lower_bound;
// }

void Netlist::remove_all_children(Decision_Node* root){
    if (root->left) remove_all_children(root->left);
    root->left = NULL;
    if (root->right) remove_all_children(root->right);
    root->right = NULL;
    if (!root->is_root) delete(root);
}

void Netlist::draw_nodes(Decision_Node* root){
    if (this->best_solution == root) root->draw_node(RED, 1.0);
    else root->draw_node(LIGHTGREY);
    if (root->left) this->draw_nodes(root->left);
    if (root->right) this->draw_nodes(root->right);
}