#include <algorithm>
using std::lower_bound;

#include <fstream>
using std::ofstream;

#include <iostream>
using std::cout;
using std::endl;

#include <sstream>
using std::ostringstream;

#include <string>
using std::string;

#include "asynchronous_algorithms/ant_colony_optimization.hxx"


//const double AntColony::PHEROMONE_DEGRADATION_RATE = 0.90;
//const double AntColony::PHEROMONE_MINIMUM = 1.0;
//const double AntColony::PHEROMONE_MAXIMUM = 20.0;

ACO_Node::ACO_Node(int l, int n) : layer(l), node(n) {
}

ACO_Node::~ACO_Node() {
    for (int i = 0; i < targets.size(); i++) {
        targets.pop_back();
    }
}

void ACO_Node::add_target(ACO_Node* aco_node) {
    pheromones.push_back(1.0);  //initialize the pheromones on all connections to 1.0
    targets.push_back(aco_node);
}

//might want to not initialize all pheromones to 1.0
void ACO_Node::add_target(ACO_Node* aco_node, double pheromone) {
    pheromones.push_back(pheromone);
    targets.push_back(aco_node);
}

ACOIndividual::ACOIndividual(double f, vector<Edge> e, vector<Edge> re) : fitness(f), edges(e), recurrent_edges(re) {
}

AntColony::AntColony(int noa, int meps, int ils, int hls, int nhl, double pdr, double p_min, double p_max) : number_of_ants(noa), input_layer_size(ils), hidden_layer_size(hls), n_hidden_layers(nhl), max_edge_population_size(meps), PHEROMONE_DEGRADATION_RATE(pdr), PHEROMONE_MINIMUM(p_min), PHEROMONE_MAXIMUM(p_max), iteration(0) {
    use_compression = false;

    n_layers = 1 + (n_hidden_layers * 2) + 1;

    pre_input = new ACO_Node(-1, 0);

    vector< ACO_Node* > input_layer;
    for (int i = 0; i < input_layer_size; i++) {
        input_layer.push_back(new ACO_Node(0, i));
    }
    neurons.push_back(input_layer);

    for (int i = 0; i < n_hidden_layers * 2; i++) {
        vector<ACO_Node*> hidden_layer;

        for (int j = 0; j < hidden_layer_size; j++) {
            hidden_layer.push_back(new ACO_Node(1 + i, j));
        }
        neurons.push_back(hidden_layer);
    }

    vector<ACO_Node*> output_layer;
    output_layer.push_back(new ACO_Node(n_layers - 1, 0));
    neurons.push_back(output_layer);

    /*
    cout << "neurons.size(): " << neurons.size() << endl;
    for (int i = 0; i < neurons.size(); i++) {
        cout << "  neurons[" << i << "].size(): " << neurons[i].size() << endl;
    }
    */

    //all the potential nodes in the neural network have been created and are in the neurons vector
    //add the connections between ACO_Nodes, there should be no connection between the output node and any other node
    for (int i = 0; i < neurons[0].size(); i++) {
        //cout << "connecting pre input to neurons[0][" << i << "]" << endl;
        pre_input->add_target(neurons[0][i]);    //connect the node before the input layer to the input layer
    }

    for (int i = 0; i < neurons.size() - 1; i++) {
        for (int j = 0; j < neurons[i].size(); j++) {
            //cout << "adding neurons to neuron[" << i << "][" << j << "]" << endl;

            if ((i % 2) == 0) { //odd layers are recurrent layers, except for the output layer
                //for hidden layer, connect to all of the next hidden layer, and one in the recurrent layer

                if (i == neurons.size() - 2) { //last hidden layer
                    for (int k = 0; k < neurons[i+1].size(); k++) {
                        //cout << "connecting neurons[" << i << "][" << j << "] to neurons[" << i+1 << "][" << k << "]" << endl;
                        neurons[i][j]->add_target(neurons[i+1][k]);
                    }
                } else {    //other hidden layers
                    for (int k = 0; k < neurons[i+2].size(); k++) {
                        //cout << "connecting neurons[" << i << "][" << j << "] to neurons[" << i+2 << "][" << k << "]" << endl;
                        neurons[i][j]->add_target(neurons[i+2][k]);
//                        neurons[i][j]->add_target(neurons[i+2][k], neurons[i+2].size());
                    }
                }

                //if not the input layer, connect to the recurrent layer for this layer
                if (i > 0) {
                    //cout << "connecting neurons[" << i << "][" << j << "] to neurons[" << i-1 << "][" << j << "]" << endl;
                    neurons[i][j]->add_target(neurons[i-1][j]);
                }
            } else {
                //for the recurrent layer, just connect to all of the next hidden layer
                for (int k = 0; k < neurons[i+1].size(); k++) {
                    //cout << "connecting neurons[" << i << "][" << j << "] to neurons[" << i+1 << "][" << k << "]" << endl;
                    neurons[i][j]->add_target(neurons[i+1][k]);
                }
            }
        }
    }
}


AntColony::~AntColony() {
    delete pre_input;

    while (neurons.size() > 0) {
        while (neurons.back().size() > 0) {
            ACO_Node *neuron = neurons.back().back();
            neurons.back().pop_back();
            delete neuron;
        }
        neurons.pop_back();
    }

    while (edge_population.size() > 0) {
        ACOIndividual *aco_individual = edge_population.back();
        edge_population.pop_back();
        delete aco_individual;
    }
}

int AntColony::get_iteration() {
    return iteration;
}

void AntColony::set_compression(bool uc) {
    use_compression = uc;
}

int AntColony::get_edge_population_size() {
    return edge_population.size();
}

double AntColony::get_best_fitness() {
    return edge_population.front()->fitness;
}

double AntColony::get_worst_fitness() {
    return edge_population.back()->fitness;
}

double AntColony::get_avg_fitness() {
    double avg = 0.0;
    for (int i = 0; i < edge_population.size(); i++) {
        avg += edge_population[i]->fitness;
    }

    return avg / edge_population.size();
}

void AntColony::compress(vector<Edge> &edges, vector<Edge> &recurrent_edges) {
    cout << "compressing edges:" << endl;
    for (int i = 0; i < edges.size(); i++) {
        cout << "    " << edges[i] << endl;
    }

    int **possible_nodes = new int*[2 + (n_hidden_layers * 2)];
    for (int i = 0; i < 2 + (n_hidden_layers * 2); i++) {
        possible_nodes[i] = new int[hidden_layer_size];
        for (int j = 0; j < hidden_layer_size; j++) {
            possible_nodes[i][j] = 0;
        }
    }

    for (int i = 0; i < edges.size(); i++) {
        possible_nodes[edges[i].src_layer][edges[i].src_node] = 1;
        possible_nodes[edges[i].dst_layer][edges[i].dst_node] = 1;
    }

    for (int i = 0; i < recurrent_edges.size(); i++) {
        possible_nodes[recurrent_edges[i].src_layer][recurrent_edges[i].src_node] = 1;
        possible_nodes[recurrent_edges[i].dst_layer][recurrent_edges[i].dst_node] = 1;
    }

    cout << "possible nodes: " << endl;
    for (int i = 0; i < (2 + (n_hidden_layers * 2)); i++) {
        for (int j = 0; j < hidden_layer_size; j++) {
            cout << " " << possible_nodes[i][j];
        }
        cout << endl;
    }

    /*
     * TODO: finish compression function
     */
}


void AntColony::get_ant_paths(vector<Edge> &edges, vector<Edge> &recurrent_edges) {
    edges.clear();
    recurrent_edges.clear();

    //generate a path through the nodes for each ant
    //make sure that the edges are ordered by layer

    vector< vector<Edge> > edges_by_layer(n_layers);
    vector< vector<Edge> > recurrent_edges_by_layer(n_layers);

    for (int ant = 0; ant < number_of_ants; ant++) {
        ACO_Node *current = pre_input;
        vector<int> visited_recurrent_layers;

//        cout << "adding edges for ant: " << ant << endl;
        //progress through the network until reaching the output node
        while (current->targets.size() > 0) {
//            cout << "calculating connection for neuron[" << current->layer << "][" << current->node << "]" << endl;

            double sum_pheromones = 0.0;
            for (int i = 0; i < current->targets.size(); i++) {
                //skip the recurrent layer if we've already been there.
                if (std::find(visited_recurrent_layers.begin(), visited_recurrent_layers.end(), current->targets[i]->layer) != visited_recurrent_layers.end()) continue;
                sum_pheromones += current->pheromones[i];
            }

            //select the target based on the pheromone distribution
            double rand = drand48() * sum_pheromones;
            for (int i = 0; i < current->targets.size(); i++) {

                if (rand < current->pheromones[i]) {
                    //skip the recurrent layer if we've already been there.
                    if (std::find(visited_recurrent_layers.begin(), visited_recurrent_layers.end(), current->targets[i]->layer) != visited_recurrent_layers.end()) continue;
                    //create an edge and update the current ACO node

                    if (current->layer >= 0) {  //only add the edges from things after the pre-input node

                        if ((current->targets[i]->layer % 2) == 0 || current->targets[i]->layer == n_layers - 1) { //connecting to hidden or output layer
//                            cout << "  adding edge: [" << current->layer << "][" << current->node << "] to [" << current->targets[i]->layer << "][" << current->targets[i]->node << "]" << endl;
                            edges_by_layer[current->layer].push_back(Edge(current->layer, current->targets[i]->layer, current->node, current->targets[i]->node));
                        } else { //recurrent layer
//                            cout << "  adding recurrent edge: [" << current->layer << "][" << current->node << "] to [" << current->targets[i]->layer << "][" << current->targets[i]->node << "]" << endl;
                            visited_recurrent_layers.push_back(current->targets[i]->layer);
                            recurrent_edges_by_layer[current->layer].push_back(Edge(current->layer, current->targets[i]->layer, current->node, current->targets[i]->node));
                        }
                    }

                    current = current->targets[i];
                    break;
                } else {
                    rand -= current->pheromones[i];
                }
            }
        }
    }

    //edges will be unordered, need to sort them
    for (int i = 0; i < edges_by_layer.size(); i++) {
        for (int j = 0; j < edges_by_layer[i].size(); j++) {

            bool edge_exists = false;
            for (int k = 0; k < edges.size(); k++) {
                if (edges[k].src_layer == edges_by_layer[i][j].src_layer &&
                    edges[k].dst_layer == edges_by_layer[i][j].dst_layer &&
                    edges[k].src_node  == edges_by_layer[i][j].src_node &&
                    edges[k].dst_node  == edges_by_layer[i][j].dst_node) {
                    edge_exists = true;
                    break;
                }
            }

            //don't add duplicate edges
            if (!edge_exists) {
                edges.push_back(edges_by_layer[i][j]);
            }
        }
    }

    for (int i = 0; i < recurrent_edges_by_layer.size(); i++) {
        for (int j = 0; j < recurrent_edges_by_layer[i].size(); j++) {

            bool recurrent_edge_exists = false;
            for (int k = 0; k < recurrent_edges.size(); k++) {
                if (recurrent_edges[k].src_layer == recurrent_edges_by_layer[i][j].src_layer &&
                    recurrent_edges[k].dst_layer == recurrent_edges_by_layer[i][j].dst_layer &&
                    recurrent_edges[k].src_node  == recurrent_edges_by_layer[i][j].src_node &&
                    recurrent_edges[k].dst_node  == recurrent_edges_by_layer[i][j].dst_node) {
                    recurrent_edge_exists = true;
                    break;
                }
            }

            //don't add duplicate recurrent_edges
            if (!recurrent_edge_exists) {
                recurrent_edges.push_back(recurrent_edges_by_layer[i][j]);
            }
        }
    }

    if (use_compression) {
        compress(edges, recurrent_edges);
    }
}


void AntColony::add_ant_paths_v(const vector<Edge> &edges) {
    for (int i = 0; i < edges.size(); i++) {
        ACO_Node *node = neurons[edges[i].src_layer][edges[i].src_node];

        for (int j = 0; j < node->targets.size(); j++) {
            if (node->targets[j]->layer == edges[i].dst_layer && node->targets[j]->node == edges[i].dst_node) {
                node->pheromones[j] += 1.0;
            }
        }
    }
}

void AntColony::add_ant_paths(double fitness, const vector<Edge> &edges, const vector<Edge> &recurrent_edges) {
    ACOIndividual *aco_individual = new ACOIndividual(fitness, edges, recurrent_edges);
    iteration++;

    if (edge_population.size() < max_edge_population_size || fitness > (*edge_population.back()).fitness) {
        vector<ACOIndividual*>::iterator it = lower_bound( edge_population.begin(), edge_population.end(), aco_individual, CompareACOIndividual() ); // find proper position in descending order
        edge_population.insert(it, aco_individual);

        if (edge_population.size() >= max_edge_population_size) {
            add_ant_paths_v(edges);
            add_ant_paths_v(recurrent_edges);
            //cout << "decreasing pheromones!" << endl;
            decrease_pheromones();
        }

        //NEED TO WRITE THIS TO A FILE
        if (iteration % 100 == 0) {
            ostringstream oss;
            oss << output_directory << "/pheromones_" << iteration;
            ofstream outfile( oss.str().c_str() );

            outfile << n_hidden_layers << " " << hidden_layer_size << " " << PHEROMONE_MINIMUM << " " << PHEROMONE_MAXIMUM << endl;
            outfile << "#pheromones" << endl;
            for (int i = 0; i < neurons.size(); i++) {
                for (int j = 0; j < neurons[i].size(); j++) {
                    outfile << "#neurons " << i << " " << j << endl;
                    for (int k = 0; k < neurons[i][j]->targets.size(); k++) {
                        outfile << "  " << neurons[i][j]->targets[k]->layer << " " << neurons[i][j]->targets[k]->node << " " << neurons[i][j]->pheromones[k] << endl;
                    }
                }
            }
            outfile.close();
        }
    }

    if (edge_population.size() >= max_edge_population_size) {
        ACOIndividual *end = edge_population.back();
        edge_population.pop_back();
        delete end;
    }

}


void AntColony::decrease_pheromones() {
    for (int i = 0; i < neurons.size(); i++) {
        for (int j = 0; j < neurons[i].size(); j++) {
            for (int k = 0; k < neurons[i][j]->targets.size(); k++) {
                neurons[i][j]->pheromones[k] *= PHEROMONE_DEGRADATION_RATE;
                //cout << "degrated neuron[" << i << "][" << j << "]->pheromones[" << k << "] to: " << neurons[i][j]->pheromones[k] << endl;

                if (neurons[i][j]->pheromones[k] < PHEROMONE_MINIMUM) neurons[i][j]->pheromones[k] = PHEROMONE_MINIMUM;
                if (neurons[i][j]->pheromones[k] > PHEROMONE_MAXIMUM) neurons[i][j]->pheromones[k] = PHEROMONE_MAXIMUM;
            }
        }
    }
}


void ant_colony_optimization(int maximum_iterations, AntColony &ant_colony, double (*objective_function)(const vector<Edge> &edges, const vector<Edge> &recurrent_edges)) {

    vector<Edge> edges;
    vector<Edge> recurrent_edges;

    for (int i = 0; i < maximum_iterations; i++) {
        //get and paths
        ant_colony.get_ant_paths(edges, recurrent_edges);

        /*
        cout << "#feed forward edges" << endl;
        for (int j = 0; j < edges.size(); j++) {
            cout << edges[j] << endl;
        }
        cout << endl;
        */

        /*
        cout << "#recurrent edges" << endl;
        for (int j = 0; j < recurrent_edges.size(); j++) {
            cout << recurrent_edges[j] << endl;
        }
        cout << endl;
        */

        double fitness = objective_function(edges, recurrent_edges);

        //if fitness improves population, add ant paths to the
        //population, removing worst.

        ant_colony.add_ant_paths(fitness, edges, recurrent_edges);

        cout << "iteration: " << i << "/" << maximum_iterations << ", new fitness: " << fitness << ", pop size: " << ant_colony.edge_population.size() << ", max pop fitness: " << ant_colony.edge_population.front()->fitness << ", min pop fitness: " << ant_colony.edge_population.back()->fitness << endl;
    }
}

void AntColony::set_output_directory(string od) {
    output_directory = od;
}

void AntColony::write_population() {
    
    for (int i = 0; i < edge_population.size(); i++) {
        ostringstream oss;
        oss << output_directory << "/" << iteration << "_" << i;

        ofstream outfile( oss.str().c_str() );

        outfile << "#hidden layers" << endl;
        outfile << n_hidden_layers << endl << endl;

        outfile << "#nodes per layer" << endl;
        outfile << hidden_layer_size << endl << endl;

        outfile << "#feed forward edges" << endl;
        for (int j = 0; j < edge_population[i]->edges.size(); j++) {
            outfile << edge_population[i]->edges[j].src_layer << " "
                    << edge_population[i]->edges[j].dst_layer << " "
                    << edge_population[i]->edges[j].src_node << " "
                    << edge_population[i]->edges[j].dst_node << " "
                    << edge_population[i]->edges[j].weight << endl;
        }
        outfile << endl;

        outfile << "#recurrent edges" << endl;
        for (int j = 0; j < edge_population[i]->recurrent_edges.size(); j++) {
            outfile << edge_population[i]->recurrent_edges[j].src_layer << " "
                    << edge_population[i]->recurrent_edges[j].dst_layer << " "
                    << edge_population[i]->recurrent_edges[j].src_node << " "
                    << edge_population[i]->recurrent_edges[j].dst_node << " "
                    << edge_population[i]->recurrent_edges[j].weight << endl;
        }

        outfile.close();
    }
}
