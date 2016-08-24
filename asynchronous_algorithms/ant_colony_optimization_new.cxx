#include <algorithm>
using std::binary_search;
using std::upper_bound;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <sstream>
using std::ostringstream;

#include <vector>
using std::vector;

#include "./asynchronous_algorithms/ant_colony_optimization_new.hxx"
#include "./neural_networks/neural_network.hxx"
#include "./neural_networks/edge_new.hxx"

void increment_pheromone(double &pheromone, double pheromone_placement_rate, double maximum_pheromone) {
    pheromone *= pheromone_placement_rate;
    if (pheromone > maximum_pheromone) pheromone = maximum_pheromone;
}

void decrement_pheromone(double &pheromone, double pheromone_degradation_rate, double minimum_pheromone) {
    pheromone *= pheromone_degradation_rate;
    if (pheromone < minimum_pheromone) pheromone = minimum_pheromone;
}

bool node_match(AntColonyNode *target, uint32_t depth, uint32_t layer, uint32_t node) {
    return target != NULL && target->get_depth() == depth && target->get_layer() == layer && target->get_node() == node;
}


AntColonyPaths::AntColonyPaths(double _fitness, const vector<EdgeNew> &_edges, const vector<EdgeNew> &_recurrent_edges) : fitness(_fitness), edges(_edges), recurrent_edges(_recurrent_edges) {
}

bool operator<(const AntColonyPaths &e1, const AntColonyPaths &e2) {
    return e1.fitness < e2.fitness;
}

bool ant_colony_paths_greater(const AntColonyPaths *e1, const AntColonyPaths *e2) {
    return e1->fitness > e2->fitness;
}



AntColonyNode::AntColonyNode(uint32_t _depth, uint32_t _layer, uint32_t _node) : depth(_depth), layer(_layer), node(_node), recurrent_pheromone(0.0) {
    recurrent_target = NULL;
}

uint32_t AntColonyNode::get_depth() const {
    return depth;
}

uint32_t AntColonyNode::get_layer() const {
    return layer;
}

uint32_t AntColonyNode::get_node() const {
    return node;
}


void AntColonyNode::connect(AntColonyNode *target) {
//    cout << "connecting to " << target << endl;
    if (target->depth == depth + 1) {
        if (recurrent_target != NULL) {
            cerr << "ERROR in file '" << __FILE__ << ":" << __LINE__ << "'." << endl;
            cerr << "\tAttempting to assign a recurrent target to an ant colony node which already has a recurrent target." << endl;
            exit(1);
        }

        if (forward_targets.size() == 0) {
            recurrent_pheromone = 1.0;
//        } else {
//            recurrent_pheromone = forward_targets.size();
        }

        recurrent_target = target;
    } else {
        forward_target_pheromones.push_back(1.0);
        forward_targets.push_back(target);

        //initialize the ant colony with an equal chance for the recurrent target being chosen as
        //any forward edge
        if (recurrent_target != NULL) {
            //recurrent_pheromone = forward_targets.size();
        }
    }
}

uint32_t AntColonyNode::get_number_connections() {
    if (recurrent_target == NULL) {
        return forward_targets.size();
    } else {
        return forward_targets.size() + 1;
    }
}

void AntColonyNode::get_connection(uint32_t conn, uint32_t &depth, uint32_t &layer, uint32_t &node) {
    if (conn < forward_targets.size()) {
        depth = forward_targets.at(conn)->depth;
        layer = forward_targets.at(conn)->layer;
        node = forward_targets.at(conn)->node;

    } else if (conn == forward_targets.size()) {
        if (recurrent_target == NULL) {
            cerr << "ERROR in file '" << __FILE__ << ":" << __LINE__ << "'." << endl;
            cerr << "\tAttempting to get a AntColonyNode connection that doesn't exist (" << conn << ")." << endl;
            cerr << "\trecurrent target exists? " << (recurrent_target == NULL) << endl;
            cerr << "\tforward_targets.size(): " << forward_targets.size() << endl;

            exit(1);
        } else {
            depth = recurrent_target->depth;
            layer = recurrent_target->layer;
            node = recurrent_target->node;
        }

    } else {
        cerr << "ERROR in file '" << __FILE__ << ":" << __LINE__ << "'." << endl;
        cerr << "\tAttempting to get a AntColonyNode connection that doesn't exist (" << conn << ")." << endl;
        cerr << "\trecurrent target exists? " << (recurrent_target == NULL) << endl;
        cerr << "\tforward_targets.size(): " << forward_targets.size() << endl;

        exit(1);
    }
}

double AntColonyNode::get_pheromone(uint32_t conn) {
    if (conn < forward_targets.size()) {
        return forward_target_pheromones.at(conn);

    } else if (conn == forward_targets.size()) {
        if (recurrent_target == NULL) {
            cerr << "ERROR in file '" << __FILE__ << ":" << __LINE__ << "'." << endl;
            cerr << "\tAttempting to get a AntColonyNode connection pheromoe that doesn't exist (" << conn << ")." << endl;
            cerr << "\trecurrent target exists? " << (recurrent_target == NULL) << endl;
            cerr << "\tforward_targets.size(): " << forward_targets.size() << endl;

            exit(1);
        } else {
            return recurrent_pheromone;
        }

    } else {
        cerr << "ERROR in file '" << __FILE__ << ":" << __LINE__ << "'." << endl;
        cerr << "\tAttempting to get a AntColonyNode connection pheromone that doesn't exist (" << conn << ")." << endl;
        cerr << "\trecurrent target exists? " << (recurrent_target == NULL) << endl;
        cerr << "\tforward_targets.size(): " << forward_targets.size() << endl;

        exit(1);
    }
}

void AntColonyNode::increment_pheromones(uint32_t dst_depth, uint32_t dst_layer, uint32_t dst_node, double pheromone_placement_rate, double maximum_pheromone) {
    if (node_match(recurrent_target, dst_depth, dst_layer, dst_node)) {
        increment_pheromone(recurrent_pheromone, pheromone_placement_rate, maximum_pheromone);
    }

    for (uint32_t i = 0; i < forward_targets.size(); i++) {
        if (node_match(forward_targets.at(i), dst_depth, dst_layer, dst_node)) {
            increment_pheromone(forward_target_pheromones.at(i), pheromone_placement_rate, maximum_pheromone);
        }
    }
}

void AntColonyNode::decrement_pheromones(uint32_t dst_depth, uint32_t dst_layer, uint32_t dst_node, double pheromone_degradation_rate, double minimum_pheromone) {
    if (node_match(recurrent_target, dst_depth, dst_layer, dst_node)) {
        decrement_pheromone(recurrent_pheromone, pheromone_degradation_rate, minimum_pheromone);
    }

    for (uint32_t i = 0; i < forward_targets.size(); i++) {
        if (node_match(forward_targets.at(i), dst_depth, dst_layer, dst_node)) {
            decrement_pheromone(forward_target_pheromones.at(i), pheromone_degradation_rate, minimum_pheromone);
        }
    }
}

/*
void AntColonyNode::decrement_pheromones(double pheromone_degradation_rate, double minimum_pheromone) {
    if (recurrent_target != NULL) {
        decrement_pheromone(recurrent_pheromone, pheromone_degradation_rate, minimum_pheromone);
    }

    for (uint32_t i = 0; i < forward_target_pheromones.size(); i++) {
        decrement_pheromone(forward_target_pheromones.at(i), pheromone_degradation_rate, minimum_pheromone);
    }
}
*/

AntColonyNode* AntColonyNode::select_path() {
    double total_pheromones = recurrent_pheromone;
    //cout << "recurrent_pheromones: " << total_pheromones << endl;

    for (uint32_t i = 0; i < forward_target_pheromones.size(); i++) {
        total_pheromones += forward_target_pheromones.at(i);
        //cout << "total_pheromones: " << total_pheromones << endl;
    }

    double rand = drand48() * total_pheromones;
    //cout << "rand: " << rand << endl;

    if (rand < recurrent_pheromone) {
        if (recurrent_target == NULL) {
            cout << "RECURRENT TARGET WAS NULL!" << endl;
            exit(1);
        }
        return recurrent_target;
    }

    rand -= recurrent_pheromone;
    for (uint32_t i = 0; i < forward_target_pheromones.size(); i++) {
        double pheromone = forward_target_pheromones.at(i);
        if (rand < pheromone) return forward_targets.at(i);
        rand -= pheromone;
    }

    //program flow should never reach here.
    cerr << "ERROR in file '" << __FILE__ << ":" << __LINE__ << "'." << endl;
    cerr << "\tCould not select the path for an ant from " << this << "." << endl;
    cerr << "\trandom number was greater than the target pheromones." << endl;
    cerr << "\tThis should never happen." << endl;
    exit(1);

    return NULL;
}

string AntColonyNode::print_pheromones() {
    ostringstream oss;

    oss << "F:";
    for (uint32_t i = 0; i < forward_target_pheromones.size(); i++) {
        oss << " " << forward_target_pheromones[i];
    }

    oss << ", R: " << recurrent_pheromone;

    return oss.str();
}

ostream& operator<< (ostream& out, const AntColonyNode &aco_node) {
    cout << "[ant colony node: depth: " << aco_node.get_depth() << ", layer: " << aco_node.get_layer() << ", node: " << aco_node.get_node() << "]";
    return out;
}

ostream& operator<< (ostream& out, const AntColonyNode *aco_node) {
    cout << "[ant colony node: depth: " << aco_node->get_depth() << ", layer: " << aco_node->get_layer() << ", node: " << aco_node->get_node() << "]";
    return out;
}

bool AntColonyNew::node_exists(uint32_t depth, uint32_t layer, uint32_t node) {
//    cout << "depth " << depth << " vs " << nodes.size() << endl;

    if (depth >= nodes.size()) {
        //cout << "returning false" << endl;
        return false;
    } else {
        //cout << "layer " << layer << " vs " << nodes.at(depth).size() << endl;

        if (layer >= nodes.at(depth).size()) {
            //cout << "returning false" << endl;
            return false;
        } else {
            //cout << "node " << node << " vs " << nodes.at(depth).at(layer).size() << endl;

            if (node >= nodes.at(depth).at(layer).size()) {
                //cout << "returning false" << endl;
                return false;
            } else {
                //cout << "returning true" << endl;
                return true;
            }
        }
    }
}

double AntColonyNew::get_best_fitness() {
    /*
    for (uint32_t i = 0; i < population.size(); i++) {
        cout << "population[" << i << "]->fitness: " << population[i]->fitness << endl;
    }
    */

    if (population.size() > 0) {
        return population.at(0)->fitness;
    } else {
        return -std::numeric_limits<double>::max();
    }
}

double AntColonyNew::get_avg_fitness() {
    if (population.size() > 0) {
        double avg = 0;
        for (uint32_t i = 0; i < population.size(); i++) {
            avg += population.at(i)->fitness;
        }
        return avg / population.size();
    } else {
        return -std::numeric_limits<double>::max();
    }
}

double AntColonyNew::get_worst_fitness() {
    if (population.size() > 0) {
        return population.at(population.size() - 1)->fitness;
    } else {
        return -std::numeric_limits<double>::max();
    }
}


uint32_t AntColonyNew::get_iteration() {
    return iteration;
}

AntColonyNew::AntColonyNew(double _pheromone_placement_rate, double _pheromone_degradation_rate, double _maximum_pheromone, double _minimum_pheromone, uint32_t _number_ants, uint32_t _recurrent_depth, uint32_t _n_input_nodes, uint32_t _n_hidden_layers, uint32_t _n_hidden_nodes, uint32_t _n_output_nodes, uint32_t _max_population_size) : pheromone_placement_rate(_pheromone_placement_rate), pheromone_degradation_rate(_pheromone_degradation_rate), maximum_pheromone(_maximum_pheromone), minimum_pheromone(_minimum_pheromone), number_ants(_number_ants), recurrent_depth(_recurrent_depth), n_input_nodes(_n_input_nodes), n_hidden_layers(_n_hidden_layers), n_hidden_nodes(_n_hidden_nodes), n_output_nodes(_n_output_nodes), max_population_size(_max_population_size), iteration(0), output_directory("./aco_output") {

    srand48(time(NULL));

    //create a starting node so ants can pick which input nodes to visit
    start_node = new AntColonyNode(0, -1, 0);

    //initialize the possible nodes for the neural networks evolved
    //by ACO
    for (uint32_t depth = 0; depth < recurrent_depth; depth++) {
        nodes.push_back( vector< vector<AntColonyNode*> >() );

        uint32_t layer = 0;
        nodes[depth].push_back( vector<AntColonyNode*>() );

        for (uint32_t node = 0; node < n_input_nodes; node++) {
            nodes.at(depth).at(layer).push_back( new AntColonyNode(depth, layer, node) );
        }

        layer = 1;
        for (; layer < n_hidden_layers + 1; layer++) {
            nodes.at(depth).push_back( vector<AntColonyNode*>() );
            for (uint32_t node = 0; node < n_hidden_nodes; node++) {
                nodes.at(depth).at(layer).push_back( new AntColonyNode(depth, layer, node) );
            }
        }

        //no recurrent layer on the output layer
        if (depth == 0) {
            nodes.at(depth).push_back( vector<AntColonyNode*>() );
            for (uint32_t node = 0; node < n_output_nodes; node++) {
                nodes.at(depth).at(layer).push_back( new AntColonyNode(depth, layer, node) );
            }
        }
    }

    //connect the starting position to the nodes in the input layer
    for (uint32_t node = 0; node < nodes.at(0).at(0).size(); node++) {
        start_node->connect(nodes.at(0).at(0).at(node));
    }

    //add possible connections between nodes
    //nodes can connect to the same node in the next recurrent layer
    //or to any node in the next layer at the same or lower depth
    for (uint32_t depth = 0; depth < nodes.size(); depth++) {
        //cout << "looping for depth: " << depth << endl;

        for (uint32_t layer = 0; layer < nodes.at(depth).size(); layer++) {
            if (depth == 0 && layer == nodes.at(depth).size() - 1) break;

            //cout << "looping for layer: " << layer << endl;
            for (uint32_t node1 = 0; node1 < nodes.at(depth).at(layer).size(); node1++) {

                //link the node to the next depth
                //the output nodes have no recurrent layer
                //cout << "[connecting to recurrent] checking to see if node exists [" << (depth + 1) << ", " << layer << ", " << node1 << "]" << endl;

                if (node_exists(depth + 1, layer, node1)) {
                    //cout << "trying to connect." << endl;
                    nodes.at(depth).at(layer).at(node1)->connect(nodes.at(depth+1).at(layer).at(node1));

                    //cout << "connecting recurrent edge: " << depth << ", " << layer << ", " << node1 << " to " << (depth + 1) << ", " << layer << ", " << node1 << endl;
                }

                //connect to the nexts layer at the same or lower depth
                for (int32_t depth2 = depth; depth2 >= 0; depth2--) {
                    if (layer + 1 >= nodes.at(depth2).size()) continue;

                    for (uint32_t node2 = 0; node2 < nodes.at(depth2).at(layer+1).size(); node2++) {

                        //cout << "[connecting to next layer] checking to see if node exists [" << depth2 << ", " << (layer+1) << ", " << node2 << "]" << endl;
                        if (node_exists(depth2, layer + 1, node2)) {
                            //cout << "trying to connect." << endl;
                            nodes.at(depth).at(layer).at(node1)->connect(nodes.at(depth2).at(layer+1).at(node2));

                            //cout << "connecting forward edge: " << depth << ", " << layer << ", " << node1 << " to " << depth2 << ", " << (layer + 1) << ", " << node2 << endl;
                        }
                    }
                }
            }
        }
    }

    /*
    cout << "finished making connectins" << endl;
    for (uint32_t i = 0; i < nodes.size(); i++) {
        cout << "depth[" << i << "/" << nodes.size() << "]" << endl;
        for (uint32_t j = 0; j < nodes[i].size(); j++) {
            cout << "    layer[" << j << "/" << nodes[i].size() << "] size: " << nodes[i][j].size() << endl;
        }   
    }   
    */

}

AntColonyNew::~AntColonyNew() {
    for (uint32_t depth = 0; depth < nodes.size(); depth++) {
        for (uint32_t layer = 0; layer < nodes.at(depth).size(); layer++) {
            for (uint32_t node = 0; node < nodes.at(depth).at(layer).size(); node++) {
                delete nodes.at(depth).at(layer).at(node);
            }
        }
    }

    while (population.size() > 0) {
        AntColonyPaths *path = population.back();
        population.pop_back();
        delete path;
    }
}

void AntColonyNew::increment_pheromones(const vector<EdgeNew> &edges, const vector<EdgeNew> &recurrent_edges) {
    for (uint32_t i = 0; i < edges.size(); i++) {
        if (edges[i].src_depth == 0 && edges[i].src_layer == 0) {
            //need to increment starting node pheromones as well
            start_node->increment_pheromones(edges[i].src_depth, edges[i].src_layer, edges[i].src_node, pheromone_placement_rate, maximum_pheromone);
        }

        nodes.at(edges[i].src_depth).at(edges[i].src_layer).at(edges[i].src_node)->increment_pheromones(edges[i].dst_depth, edges[i].dst_layer, edges[i].dst_node, pheromone_placement_rate, maximum_pheromone);
    }

    for (uint32_t i = 0; i < recurrent_edges.size(); i++) {
        nodes.at(recurrent_edges[i].src_depth).at(recurrent_edges[i].src_layer).at(recurrent_edges[i].src_node)->increment_pheromones(recurrent_edges[i].dst_depth, recurrent_edges[i].dst_layer, recurrent_edges[i].dst_node, pheromone_placement_rate, maximum_pheromone);

//        cout << "incrementing recurrent edge: " << recurrent_edges[i].src_depth << ", " << recurrent_edges[i].src_layer << ", " << recurrent_edges[i].src_node << " to " << recurrent_edges[i].dst_depth << ", " << recurrent_edges[i].dst_layer << ", " << recurrent_edges[i].dst_node << endl;
    }
}

void AntColonyNew::print_pheromones() {
    cout << "start node: " << start_node->print_pheromones() << endl;

    for (uint32_t depth = 0; depth < nodes.size(); depth++) {
        cout << "depth: " << depth << endl;
        for (uint32_t layer = 0; layer < nodes.at(depth).size(); layer++) {
            cout << "    layer: " << layer<< endl;
            for (uint32_t node = 0; node < nodes.at(depth).at(layer).size(); node++) {
                cout << "        node " << node << ": " << nodes.at(depth).at(layer).at(node)->print_pheromones() << endl;
            }
        }
    }
}


void AntColonyNew::add_ant_paths(double fitness, const vector<EdgeNew> &edges, const vector<EdgeNew> &recurrent_edges) {
    bool new_best = false;

    new_best = (population.size() > 0 && fitness > population[population.size() - 1]->fitness);

    if (population.size() < max_population_size || new_best) {
        //if the new edges are the best found, write to output directory
        if (population.size() > 0 && fitness > population[0]->fitness) {
            ostringstream oss;
            oss << output_directory << "/" << iteration;

            NeuralNetwork *nn = new NeuralNetwork(recurrent_depth, n_input_nodes, n_hidden_layers, n_hidden_nodes, n_output_nodes, "linear");
            nn->set_edges(edges, recurrent_edges, input_labels, output_labels);
            nn->write_to_file(oss.str());
            delete nn;
        }

        AntColonyPaths *paths = new AntColonyPaths(fitness, edges, recurrent_edges);

        population.insert(upper_bound(population.begin(), population.end(), paths, ant_colony_paths_greater), paths);

        if (population.size() > max_population_size) {
            increment_pheromones(edges, recurrent_edges);
            AntColonyPaths *last_path = population.back();
            population.pop_back();
            delete last_path;

            print_pheromones();
        }
    }

    if (!new_best) decrement_pheromones(edges, recurrent_edges);
    iteration++;
}

void AntColonyNew::decrement_pheromones(const vector<EdgeNew> &edges, const vector<EdgeNew> &recurrent_edges) {
    for (uint32_t i = 0; i < edges.size(); i++) {
        if (edges[i].src_depth == 0 && edges[i].src_layer == 0) {
            //need to increment starting node pheromones as well
            start_node->decrement_pheromones(edges[i].src_depth, edges[i].src_layer, edges[i].src_node, pheromone_degradation_rate, minimum_pheromone);
        }

        nodes.at(edges[i].src_depth).at(edges[i].src_layer).at(edges[i].src_node)->decrement_pheromones(edges[i].dst_depth, edges[i].dst_layer, edges[i].dst_node, pheromone_degradation_rate, minimum_pheromone);
    }

    for (uint32_t i = 0; i < recurrent_edges.size(); i++) {
        nodes.at(recurrent_edges[i].src_depth).at(recurrent_edges[i].src_layer).at(recurrent_edges[i].src_node)->decrement_pheromones(recurrent_edges[i].dst_depth, recurrent_edges[i].dst_layer, recurrent_edges[i].dst_node, pheromone_degradation_rate, minimum_pheromone);

//        cout << "decrementing recurrent edge: " << recurrent_edges[i].src_depth << ", " << recurrent_edges[i].src_layer << ", " << recurrent_edges[i].src_node << " to " << recurrent_edges[i].dst_depth << ", " << recurrent_edges[i].dst_layer << ", " << recurrent_edges[i].dst_node << endl;
    }
}


/*
void AntColonyNew::decrement_pheromones() {
    for (uint32_t depth = 0; depth < nodes.size(); depth++) {
        for (uint32_t layer = 0; layer < nodes.at(depth).size(); layer++) {
            for (uint32_t node = 0; node < nodes.at(depth).at(layer).size(); node++) {
                nodes.at(depth).at(layer).at(node)->decrement_pheromones(pheromone_degradation_rate, minimum_pheromone);
            }
        }
    }
}
*/

void AntColonyNew::generate_neural_network(vector<EdgeNew> &edges, vector<EdgeNew> &recurrent_edges) {
    edges.clear();
    recurrent_edges.clear();

    //all ants start at the starting node
    vector<AntColonyNode*> ant_positions;
    ant_positions.reserve(number_ants);
    for (uint32_t ant = 0; ant < number_ants; ant++) {
        ant_positions.push_back( start_node->select_path() );
//        cout << "ant_positions[" << ant << "]: " << ant_positions.at(ant) << endl;
    }

    for (uint32_t layer = 0; layer < nodes.at(0).size() - 1; layer++) {
        for (uint32_t ant = 0; ant < ant_positions.size(); ant++) {
            AntColonyNode *current_position = ant_positions.at(ant);
//            cout << "got current position: " << current_position << endl;

            AntColonyNode *next_position = current_position->select_path();
//            cout << "got next position: " << next_position << endl;

            while (next_position->get_layer() != current_position->get_layer() + 1) {
                //this was a recurrent edge
                EdgeNew edge(current_position->get_depth(), current_position->get_layer(), current_position->get_node(), next_position->get_depth(), next_position->get_layer(), next_position->get_node());
                //cout << "creating recurrent edge: " << edge << endl;

                //insert the edges in order so that we can just run through them in the neural network
                //dont add duplicates
                if (!binary_search(recurrent_edges.begin(), recurrent_edges.end(), edge)) {
                    recurrent_edges.insert(upper_bound(recurrent_edges.begin(), recurrent_edges.end(), edge), edge);
                }

                current_position = next_position;
                next_position = current_position->select_path();
                //cout << "got next position: " << next_position << ", nodes.size(): " << nodes.size() << endl;
            }

            //this was a forward moving edge
            //insert the edges in order, don't add duplicate
            EdgeNew edge(current_position->get_depth(), current_position->get_layer(), current_position->get_node(), next_position->get_depth(), next_position->get_layer(), next_position->get_node());

            //cout << "creating forward edge: " << edge << endl;

            if (!binary_search(edges.begin(), edges.end(), edge)) {
                edges.insert(upper_bound(edges.begin(), edges.end(), edge), edge);
            }

            ant_positions.at(ant) = next_position;
        }
    }
}

void AntColonyNew::generate_fully_connected_neural_network(vector<EdgeNew> &edges, vector<EdgeNew> &recurrent_edges) {
    edges.clear();
    recurrent_edges.clear();

    uint32_t dst_depth, dst_layer, dst_node;

    for (uint32_t depth = 0; depth < nodes.size(); depth++) {
        for (uint32_t layer = 0; layer < nodes.at(depth).size(); layer++) {
            for (uint32_t node = 0; node < nodes.at(depth).at(layer).size(); node++) {

                uint32_t n_connections = nodes.at(depth).at(layer).at(node)->get_number_connections();
                for (uint32_t i = 0; i < n_connections; i++) {
                    nodes.at(depth).at(layer).at(node)->get_connection(i, dst_depth, dst_layer, dst_node);
                    double pheromones = nodes.at(depth).at(layer).at(node)->get_pheromone(i);

                    if (dst_depth > depth) recurrent_edges.push_back(EdgeNew(depth, layer, node, dst_depth, dst_layer, dst_node, pheromones));
                    else edges.push_back(EdgeNew(depth, layer, node, dst_depth, dst_layer, dst_node, pheromones));
                }

            }
        }
    }
}


void AntColonyNew::set_labels(const vector<string> &_input_labels, const vector<string> &_output_labels) {
    input_labels = _input_labels;
    output_labels = _output_labels;
}
void AntColonyNew::set_output_directory(string od) {
    output_directory = od;
}

void AntColonyNew::write_population() {

    for (int i = 0; i < population.size(); i++) {
        ostringstream oss;
        oss << output_directory << "/" << iteration << "_" << i;

        NeuralNetwork *nn = new NeuralNetwork(recurrent_depth, n_input_nodes, n_hidden_layers, n_hidden_nodes, n_output_nodes, "linear");
        nn->set_edges(population.at(i)->edges, population.at(i)->recurrent_edges, input_labels, output_labels);

        nn->write_to_file(oss.str());

        delete nn;
    }       
}

