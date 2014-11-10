#include <algorithm>
using std::lower_bound;

#include <iostream>
using std::cout;
using std::endl;

#include "asynchronous_algorithms/ant_colony_optimization.hxx"


const double AntColony::PHEROMONE_DEGRADATION_RATE = 0.95;
const double AntColony::PHEROMONE_MINIMUM = 1.0;
const double AntColony::PHEROMONE_MAXIMUM = 10.0;

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

ACOIndividual::ACOIndividual(double f, vector<Edge> e, vector<Edge> re) : fitness(f), edges(e), recurrent_edges(re) {
}

AntColony::AntColony(int noa, int meps, int ils, int hls, int nhl) : number_of_ants(noa), input_layer_size(ils), hidden_layer_size(hls), n_hidden_layers(nhl), max_edge_population_size(meps) {
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

int AntColony::get_edge_population_size() {
    return edge_population.size();
}

double AntColony::get_best_fitness() {
    return edge_population.front()->fitness;
}

double AntColony::get_worst_fitness() {
    return edge_population.back()->fitness;
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

//        cout << "adding edges for ant: " << ant << endl;
        //progress through the network until reaching the output node
        while (current->targets.size() > 0) {
//            cout << "calculating connection for neuron[" << current->layer << "][" << current->node << "]" << endl;

            double sum_pheromones = 0.0;
            for (int i = 0; i < current->targets.size(); i++) {
                sum_pheromones += current->pheromones[i];
            }

            //select the target based on the pheromone distribution
            double rand = drand48() * sum_pheromones;
            for (int i = 0; i < current->targets.size(); i++) {
                if (rand < current->pheromones[i]) {
                    //create an edge and update the current ACO node

                    if (current->layer >= 0) {  //only add the edges from things after the pre-input node

                        if ((current->targets[i]->layer % 2) == 0 || current->targets[i]->layer == n_layers - 1) { //connecting to hidden or output layer
//                            cout << "  adding edge: [" << current->layer << "][" << current->node << "] to [" << current->targets[i]->layer << "][" << current->targets[i]->node << "]" << endl;
                            edges_by_layer[current->layer].push_back(Edge(current->layer, current->targets[i]->layer, current->node, current->targets[i]->node));
                        } else { //recurrent layer
//                            cout << "  adding recurrent edge: [" << current->layer << "][" << current->node << "] to [" << current->targets[i]->layer << "][" << current->targets[i]->node << "]" << endl;
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

    if (edge_population.size() < max_edge_population_size || fitness > (*edge_population.back()).fitness) {
        vector<ACOIndividual*>::iterator it = lower_bound( edge_population.begin(), edge_population.end(), aco_individual, CompareACOIndividual() ); // find proper position in descending order
        edge_population.insert(it, aco_individual);

        if (edge_population.size() >= max_edge_population_size) {
            add_ant_paths_v(edges);
            add_ant_paths_v(recurrent_edges);
            decrease_pheromones();
        }

        cout << "#pheromones" << endl;
        for (int i = 0; i < neurons.size(); i++) {
            for (int j = 0; j < neurons[i].size(); j++) {
                cout << "#neurons[" << i << "][" << j << "]:" << endl;
                for (int k = 0; k < neurons[i][j]->targets.size(); k++) {
                    cout << "  [" << neurons[i][j]->targets[k]->layer << "][" << neurons[i][j]->targets[k]->node << "]: " << neurons[i][j]->pheromones[k] << endl;
                }
            }
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
