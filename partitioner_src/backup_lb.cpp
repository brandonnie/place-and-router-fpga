int Netlist::LB(Decision_Node* curr_node){
    //return this->compute_cut_size(curr_node);
    vector<vector<int> > partitions = this->compute_partition(curr_node);
    if (!partitions[0].size() || !partitions[1].size()){
        return 0;
    }
    int lower_bound = this->compute_cut_size(partitions);
    int max_cut_increase = 0;
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
        int cut_increase = 0;
        if (left_cut_size < right_cut_size){
            lower_bound += left_cut_size - lower_bound;
        } else {
            lower_bound += right_cut_size - lower_bound;
        }
    }
    // if (lower_bound > init_lb){
    //     cout << "Lower bound increased from " << init_lb << " to " << lower_bound << endl;
    // }
    return lower_bound;
}