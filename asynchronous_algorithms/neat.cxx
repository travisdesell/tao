#include <algorithm>
using std::lower_bound;

#include <cmath>

#include <limits>

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include "asynchronous_algorithms/neat.hxx"

#include "neural_networks/edge.hxx"


double get_random_weight() {
    return (drand48() * 2.0) - 1.0;
}

NEATGene::NEATGene(bool en, double we, int in, int on, int inn) : enabled(en), weight(we), input_node(in), output_node(on), innovation(inn) {
}

NEATGene::NEATGene(NEATGene *parent) : enabled(parent->enabled), weight(parent->weight), input_node(parent->input_node), output_node(parent->output_node), innovation(parent->innovation) {
}

NEATGene::NEATGene(bool en, double we, int in, int on, vector<NEATGene*> &innovations) : enabled(en), weight(we), input_node(in), output_node(on) {
    //get the innovation number for this NEATGene

    bool innovation_found = false;
    for (int i = 0; i < innovations.size(); i++) {
        if (innovations[i]->input_node == input_node && innovations[i]->output_node == output_node) {
            innovation = i;
            innovation_found = true;
            break;
        }
    }

    //this gene wasn't in the innovations, so add it
    if (!innovation_found) {
        innovation = innovations.size();
        innovations.push_back(this);
    }
}

ostream& operator<< (ostream& out, const NEATGene *gene) {
    out << "[gene: enabled? " << gene->enabled 
        << ", weight: " << gene->weight
        << ", input_node: " << gene->input_node
        << ", output_node: " << gene->output_node
        << ", innovation: " << gene->innovation
        << "]";

    return out;
}

NEATIndividual::NEATIndividual() : fitness(0), species(-1), champion_iterations(0) {
}

NEATIndividual::NEATIndividual(NEATIndividual *parent) : fitness(parent->fitness), species(parent->species), champion_iterations(parent->champion_iterations), node_genes(parent->node_genes) {
    for (int i = 0; i < parent->connection_genes.size(); i++) {
        connection_genes.push_back(new NEATGene(parent->connection_genes[i]));
    }
}

void NEATIndividual::add_connection_gene(NEATGene* gene) {
    connection_genes.push_back(gene);
}

void NEATIndividual::get_edges(int n_input_nodes, int n_output_nodes, int &n_hidden_layers, int &nodes_per_layer, const vector<NEATNode*> &all_nodes, const vector< vector<NEATNode*> > &all_nodes_by_layer, vector<Edge> &edges, vector<Edge> &recurrent_edges) {
    //need to add edges, input to output, based on enabled genes

    edges.clear();
    recurrent_edges.clear();

    n_hidden_layers = (all_nodes_by_layer.size() - 2) / 2; //subtract input and output layers
    nodes_per_layer = 0;
    for (int i = 1; i < all_nodes_by_layer.size() - 1; i++) {
        if (nodes_per_layer < all_nodes_by_layer[i].size()) {
            nodes_per_layer = all_nodes_by_layer[i].size();
        }
    }

    //all hidden nodes in a single layer
    //one output node
    //a set of input nodes
    //odd layers are recurrent layers
    for (int i = 0; i < connection_genes.size(); i++) {
        if (!connection_genes[i]->enabled) continue;

        int input_node = connection_genes[i]->input_node;
        int output_node = connection_genes[i]->output_node;

        NEATNode *input = all_nodes[input_node];
        NEATNode *output = all_nodes[output_node];

        if (input->layer < output->layer) {
            edges.push_back(Edge(input->layer, output->layer, input->node, output->node, connection_genes[i]->weight));
        } else {
            recurrent_edges.push_back(Edge(input->layer, output->layer, input->node, output->node, connection_genes[i]->weight));
        }
//        cout << "connection: " << connection_genes[i] << endl;
//        cout << "pushed back: " << edges.back() << endl;
    }

}

void NEATIndividual::set_node_genes() {
    for (int i = 0; i < connection_genes.size(); i++) {
        int node = connection_genes[i]->input_node;

         if (node_genes.size() == 0) {
                node_genes.push_back(node);
         } else {
            vector<int>::iterator it = std::lower_bound(node_genes.begin(), node_genes.end(), node);

            //DOES THIS CHECK IF THE NODE ALREADY EXISTS??
            if (*it != node) {
                node_genes.insert(it, node);
            }
         }

        node = connection_genes[i]->output_node;

        vector<int>::iterator it = std::lower_bound(node_genes.begin(), node_genes.end(), node);
        if (*it != node) {
            node_genes.insert(it, node);
        }
    }
}


void print_individual(ostream &out, const NEATIndividual *individual) {
    out << "INDIVIDUAL - fitness: " << individual->fitness
        << ", species: " << individual->species
        << ", champion iterations: " << individual->champion_iterations
        << ", node_genes: (";
    for (int i = 0; i < individual->node_genes.size(); i++) {
        out << " " << individual->node_genes[i];
    }
    out << " )\n";
    for (int i = 0; i < individual->connection_genes.size(); i++) {
        out << "\t" << individual->connection_genes[i] << "\n";
    }
}

ostream& operator<< (ostream& out, const NEATIndividual *individual) {
    print_individual(out, individual);
    return out;
}


NEAT::NEAT(double ew, double dw, double ww, double ct, double norm, double mwcr, double wmr, double anmr, double almr, double icr, double cwar, double rwmr, double uwmr, double up, double eibpd, int ps) : last_innovation(0), excess_weight(ew), disjoint_weight(dw), weight_weight(ww), compatibility_threshold(ct), normalization(norm), mutation_without_crossover_rate(mwcr), weight_mutation_rate(wmr), add_node_mutation_rate(anmr), add_link_mutation_rate(almr), interspecies_crossover_rate(icr), crossover_weight_average_rate(cwar), random_weight_mutation_rate(rwmr), uniform_weight_mutation_rate(uwmr), uniform_perturbation(up), enable_if_both_parents_disabled(eibpd), population_size(ps) {
}

NEATNode::NEATNode(int i, int l, int n) : id(i), layer(l), node(n) {
}

NEATIndividual* NEAT::mutate_add_link(NEATIndividual *parent) {
//    cout << "ADDING LINK!" << endl;
    NEATIndividual *result = new NEATIndividual(parent);

    //WHAT IF LAYER HAS NO NODES?
    //swap nodes if input layer < output layer
    NEATNode* input_node = all_nodes[parent->node_genes[ parent->node_genes.size() * drand48() ]];
    NEATNode* output_node = all_nodes[parent->node_genes[ parent->node_genes.size() * drand48() ]];
    while (input_node->layer == output_node->layer) {
        input_node = all_nodes[parent->node_genes[ parent->node_genes.size() * drand48() ]];
        output_node = all_nodes[parent->node_genes[ parent->node_genes.size() * drand48() ]];
    }

    if (input_node->layer > output_node->layer) {
        NEATNode *tmp = input_node;
        input_node = output_node;
        output_node = tmp;
    }

    int input_node_id = input_node->id;
    int output_node_id = output_node->id;

    //dont add links that already exists
    bool edge_exists = false;
    for (int i = 0; i < innovations.size(); i++) {
        if (innovations[i]->input_node == input_node_id && innovations[i]->output_node == output_node_id) {
            edge_exists = true;
            break;
        }
    }

    if (!edge_exists) {
        NEATGene *new_connection = new NEATGene(true, get_random_weight(), input_node_id, output_node_id, innovations);

        result->add_connection_gene(new_connection);
    } else {
//        cout << "LINK ALREADY EXISTS!" << endl;
    }

    return result;
}

NEATIndividual* NEAT::mutate_add_node(NEATIndividual *parent) {
//    cout << "ADDING NODE!!" << endl;
    //add a new node in between the nodes of the conncetion
    //disable the parent
    //first child gets a weight of 1.0, second gets the parent's weight
    NEATIndividual *result = new NEATIndividual(parent);

    NEATGene *connection_to_split = result->connection_genes[ result->connection_genes.size() * drand48() ];
    while (!connection_to_split->enabled) {
        //don't split a disabled link
        connection_to_split = result->connection_genes[ result->connection_genes.size() * drand48() ];
    }
    connection_to_split->enabled = false;

    NEATNode* input_node = all_nodes[connection_to_split->input_node];
    NEATNode* output_node = all_nodes[connection_to_split->output_node];

    NEATNode* new_node;

    if (output_node->layer - input_node->layer < 2) {
//        cout << "no middle layer, adding new layers" << endl;
        //no layer between output and input layer, need to add one
        all_nodes_by_layer.insert(all_nodes_by_layer.begin() + input_node->layer + 1, vector<NEATNode*>());
        all_nodes_by_layer.insert(all_nodes_by_layer.begin() + input_node->layer + 2, vector<NEATNode*>());

        new_node = new NEATNode(all_nodes.size(), input_node->layer + 2, 0);
        all_nodes.push_back(new_node);
        all_nodes_by_layer[input_node->layer + 2].push_back(new_node);

    } else {
//        cout << "middle layer, not adding new layers" << endl;
        // there is a layer between output and input, don't need to add one
        // add it in the row after the input node
        new_node = new NEATNode(all_nodes.size(), input_node->layer + 2, all_nodes_by_layer[input_node->layer+2].size());
        all_nodes.push_back(new_node);
        all_nodes_by_layer[input_node->layer+2].push_back(new_node);
    }

//    cout << "updating all nodes by layer" << endl;
    for (int i = 0; i < all_nodes_by_layer.size(); i++) {
        for (int j = 0; j < all_nodes_by_layer[i].size(); j++) {
            all_nodes_by_layer[i][j]->layer = i;
            all_nodes_by_layer[i][j]->node = j;
        }
    }

//    cout << "making new genes" << endl;

    NEATGene *first = new NEATGene(true, 1.0, connection_to_split->input_node, new_node->id, innovations);
    NEATGene *second = new NEATGene(true, connection_to_split->weight, new_node->id, connection_to_split->output_node, innovations);

    /*
    cout << "NEW GENES:" << endl;
    cout << "\t" << first << endl;
    cout << "\t" << second << endl;
    */

    result->add_connection_gene(first);
    result->add_connection_gene(second);
    return result;
}

NEATIndividual* NEAT::mutate_weights(NEATIndividual *parent) {
    NEATIndividual *child = new NEATIndividual(parent);

    for (int i = 0; i < child->connection_genes.size(); i++) {
        double randval = drand48();
        if (randval < random_weight_mutation_rate) {
            child->connection_genes[i]->weight = get_random_weight();
        } else if (randval < random_weight_mutation_rate + uniform_weight_mutation_rate) {
            double randval2 = drand48();
            if (randval2 < 0.5) {
                child->connection_genes[i]->weight += uniform_perturbation;
            } else {
                child->connection_genes[i]->weight -= uniform_perturbation;
            }
        }
    }

    return child;
}

NEATIndividual* NEAT::crossover(NEATIndividual *p1, NEATIndividual *p2) {
    //Disabled genes have a 25% chance of being reenabled during crossover, allowing networks to make use of older genes once again.

    //Matching genes are inherited randomly, whereas disjoint genes (those that do not match in the middle) and excess genes (those that do not match in the end) are inherited from the more fit parent. In this case, equal fitnesses are assumed, so the disjoint and excess genes are also inherited randomly.

    //In composing the offspring, genes are randomly chosen from either parent at matching genes, whereas all excess or disjoint genes are always included from the more fit parent. This way, historical markings allow NEAT to perform crossover using linear genomes without the need for expensive topological analysis.
    NEATIndividual* new_individual = new NEATIndividual();

    int i = 0, j = 0;
    while (i < p1->connection_genes.size() && j < p2->connection_genes.size()) {
        if (i >= p1->connection_genes.size()) {
            if (p1->fitness == p2->fitness) {
                //p2's gene is excess, fitnesses are equal so add randomly
                if (drand48() < 0.5) {
                    new_individual->add_connection_gene( p2->connection_genes[j] );
                }

            } else if (p2->fitness > p1->fitness) {
                //p2 is more fit and gene is excess, add it
                new_individual->add_connection_gene( p2->connection_genes[j] );
            }
            j++;

        } else if (j >= p2->connection_genes.size()) {
            if (p1->fitness == p2->fitness) {
                //p2's gene is excess, fitnesses are equal so add randomly
                if (drand48() < 0.5) {
                    new_individual->add_connection_gene( p1->connection_genes[i] );
                }

            } else if (p1->fitness > p2->fitness) {
                //p2 is more fit and gene is excess, add it
                new_individual->add_connection_gene( p1->connection_genes[i] );
            }
            i++;

        } else {
            if (p1->connection_genes[i]->innovation == p2->connection_genes[j]->innovation) {
                //same gene, pick one randomly
                if (drand48() < 0.5) {
                    new_individual->add_connection_gene( p1->connection_genes[i] );
                } else {
                    new_individual->add_connection_gene( p2->connection_genes[j] );
                }
                if (drand48() < crossover_weight_average_rate) {
                    //make the weight the average 40% of the time
                    double p1_weight = p1->connection_genes[i]->weight;
                    double p2_weight = p2->connection_genes[j]->weight;
                    double average = (p1_weight + p2_weight) / 2.0;

                    new_individual->connection_genes.back()->weight = average;
                }
                i++;
                j++;

            } else if (p1->connection_genes[i]->innovation < p2->connection_genes[j]->innovation) {
                //p1 has a disjoint gene
                if (p1->fitness == p2->fitness) {
                    //add it at random
                    if (drand48() < 0.5) {
                        new_individual->add_connection_gene( p1->connection_genes[i] );
                    }

                } else if (p1->fitness > p2->fitness) {
                    //parent 1 is more fit, add it
                    new_individual->add_connection_gene( p1->connection_genes[i] );
                }

                i++;
            } else {
                //p2 has a disjoint gene
                if (p1->fitness == p2->fitness) {
                    //add it at random
                    if (drand48() < 0.5) {
                        new_individual->add_connection_gene( p2->connection_genes[j] );
                    }

                } else if (p2->fitness > p1->fitness) {
                    //parent 2 is more fit, add it
                    new_individual->add_connection_gene( p2->connection_genes[j] );
                }

                j++;
            }
        }
    }

    //need to re-enable genes if they were disabled at a random chance
    for (int i = 0; i < new_individual->connection_genes.size(); i++) {
        if (!new_individual->connection_genes[i]->enabled) {
            if (drand48() < enable_if_both_parents_disabled) {
                new_individual->connection_genes[i]->enabled = true;
            }
        }
    }

    //need to insert these in order without duplicates
    new_individual->set_node_genes();
    return new_individual;
}

void NEAT::assign_species() {
    //clear out the old species
    while (species.size() > 0) {
        while (species.back().size() > 0) {
            species.back().pop_back();  //don't delete the NEATIndividual because its also in the population
        }
        species.pop_back();
    }

    //assign the individuals to the population
    for (int i = 0; i < population.size(); i++) {
        if (species.size() == 0) {  //create the first species
            vector<NEATIndividual*> new_species;
            new_species.push_back(population[i]);
            species.push_back(new_species);
            population[i]->species = species.size() - 1;

        } else {
            bool matches_species = false;
            for (int j = 0; j < species.size(); j++) {
                //calculate the disjoint_count, excess_count, and avg_weight_difference
                double disjoint_count = 0.0;
                double excess_count = 0.0;
                int avg_weight_count = 0.0;
                double avg_weight_difference = 0.0;

                NEATIndividual *representative = species[j][0];
                int ri = 0, pi = 0;
                while (ri < representative->connection_genes.size() || pi < population[i]->connection_genes.size()) {
                    if (ri >= representative->connection_genes.size()) {
                        excess_count++;
                        pi++;
                        continue;
                    }

                    if (pi >= population[i]->connection_genes.size()) {
                        excess_count++;
                        ri++;
                        continue;
                    }

                    int rep_innovation = representative->connection_genes[ri]->innovation;
                    int pop_innovation = population[i]->connection_genes[pi]->innovation;

                    if (rep_innovation == pop_innovation) {
                        avg_weight_count++;
                        avg_weight_difference += fabs(representative->connection_genes[ri]->weight - population[i]->connection_genes[pi]->weight);
                        ri++;
                        pi++;
                    } else if (rep_innovation < pop_innovation) {
                        ri++;
                        disjoint_count++;
                    } else {
                        pi++;
                        disjoint_count++;
                    }
                }

                avg_weight_difference /= avg_weight_count;

                double compatibility = (excess_weight * excess_count) / normalization;
                compatibility += (disjoint_weight * disjoint_count) / normalization;
                compatibility += weight_weight * avg_weight_difference;

//                cout << "compatibility: " << compatibility << ", representitive n_genes: " << representative->connection_genes.size() << ", population[" << i << "] n_genes: " << population[i]->connection_genes.size() << ", excess weight: " << excess_weight << ", excess count: " << excess_count << ", disjoint weight: " << disjoint_weight << ", disjoint count: " << disjoint_count << ", weight weight: " << weight_weight << ", avg_weigh_difference: " << avg_weight_difference << endl;

                if (compatibility < compatibility_threshold) {
                    species[j].push_back(population[i]);
                    population[i]->species = j;
                    matches_species = true;
                    break;
                }
            }

            if (!matches_species) {
                vector<NEATIndividual*> new_species;
                new_species.push_back(population[i]);
                species.push_back(new_species);
                population[i]->species = species.size() - 1;
            }
        }

//        cout << population[i] << endl;
    }
}

NEATIndividual* NEAT::interspecies_crossover() {
    int first, second;
    first = population.size() * drand48();
    second = (population.size() - 1) * drand48();
    if (second == first) second++;

    NEATIndividual *f, *s;
    f = population[first];
    s = population[second];
    return crossover(f, s);
}

NEATIndividual* NEAT::intraspecies_crossover() {

    //select a species at random
    int random_species = species.size() * drand48();
    if (species[random_species].size() == 1) return new NEATIndividual(species[random_species][0]);

    //select two individuals within the species
    int first, second;
    first = species[random_species].size() * drand48();
    second = (species[random_species].size() - 1) * drand48();
    if (second == first) second++;

    NEATIndividual *f, *s;
    f = species[random_species][first];
    s = species[random_species][second];
    return crossover(f, s);
}

bool NEAT::is_champion(NEATIndividual *individual) {
    int s = individual->species;
    for (int i = 0; i < species[s].size(); i++) {
        if (species[s][i]->fitness > individual->fitness) return false;
    }
    return true;
}

void NEAT::initialize_population(int n_input_nodes, int n_output_nodes) {
    all_nodes_by_layer.resize(2);

    all_nodes_by_layer[0].resize(n_input_nodes);
    all_nodes_by_layer[1].resize(n_output_nodes);

    int id;
    for (int i = 0; i < n_input_nodes; i++) {
        id = all_nodes.size();
        NEATNode *node = new NEATNode(id, 0, i);
        all_nodes.push_back(node);
        all_nodes_by_layer[0][i] = node;
    }

    for (int i = 0; i < n_output_nodes; i++) {
        id = all_nodes.size();
        NEATNode *node = new NEATNode(id, 1, i);
        all_nodes.push_back(node);
        all_nodes_by_layer[1][i] = node;
    }

    for (int k = 0; k < population_size; k++) {
        NEATIndividual *current = new NEATIndividual();

        for (int i = 0; i < n_input_nodes; i++) {
            for (int j = 0; j < n_output_nodes; j++) {
                current->add_connection_gene(new NEATGene(true, get_random_weight(), i, n_input_nodes + j, innovations));
            }
        }
        current->set_node_genes();

        population.push_back(current);

    }
}

void NEAT::next_population() {
    vector< NEATIndividual* > next_population;

    vector<int> stagnant_species;
    int max_stagnation = 50;

    //directly copy over champions from species with at least N individuals
    for (int k = 0; k < population_size; k++) {
        if (is_champion(population[k])) {
            if (population[k]->champion_iterations >= max_stagnation) {
                stagnant_species.push_back(population[k]->species);
                continue;
            }
            NEATIndividual *copy = new NEATIndividual(population[k]);
            copy->champion_iterations++;
            next_population.push_back(copy);
//            cout << "champion: " << copy << endl;
        } else {
            population[k]->champion_iterations = 0;
        }
    }

    //remove stagnant species from population
    for (int i = 0; i < stagnant_species.size(); i++) {
        cout << "removing species: " << stagnant_species[i] << endl;

        vector< NEATIndividual* >::iterator it = population.begin();

        while (it != population.end()) {
            if ( (*it)->species == stagnant_species[i] ) {
                population.erase(it);
            } else {
                it++;
            }
        }

        cout << "population.size(): " << population.size() << endl;
    }

    if (species.size() > 10) compatibility_threshold = 2.3;
    if (species.size() < 10) compatibility_threshold = 1.7;
    if (species.size() == 10) compatibility_threshold = 2.0;

    while (next_population.size() < population_size) {
        //options:
        //  mutation without crossover 25%
        //  mutate weights      80% per genome
        //      uniform         90%
        //      random          10%
        //  mutate add node      1%
        //  mutate add link     10%
        //
        //
        //  crossover           75% ???
        //      interspecies    5%
        //      intraspecies    95%
        //
        //      40% average of parent weights
        //      otherwise from most fit parent (or random from parent if fitnesses equal)

        double randval = drand48();
        if (randval < mutation_without_crossover_rate) {
            NEATIndividual *parent = population[drand48() * population.size()];

            randval = drand48();
            if (randval < weight_mutation_rate) {
                next_population.push_back( mutate_weights(parent) );
            } else if (randval < add_node_mutation_rate + weight_mutation_rate) {
                next_population.push_back( mutate_add_node(parent) );
            } else if (randval < add_link_mutation_rate + add_node_mutation_rate + weight_mutation_rate) {
                next_population.push_back( mutate_add_link(parent) );
            } else {
                //??????
            }

        } else {    //perform crossover
            randval = drand48();
            if (randval < interspecies_crossover_rate) {
                next_population.push_back( interspecies_crossover() );
            } else {
                next_population.push_back( intraspecies_crossover() );
            }
        }
    }

    //clean up the old population
    vector< NEATIndividual* > old_population = population;
    population = next_population;

    while (!old_population.empty()) {
        NEATIndividual *current = old_population.back();
        old_population.pop_back();
        delete current;
    }
}

void NEAT::iterate(int max_iterations, int n_input_nodes, int n_output_nodes, double (*objective_function)(int n_hidden_layers, int nodes_per_layer, const vector<Edge> &edges, const vector<Edge> &recurrent_edges)) {
    int n_hidden_layers;
    int nodes_per_layer;
    vector<Edge> edges;
    vector<Edge> recurrent_edges;

    initialize_population(n_input_nodes, n_output_nodes);

    double min_fitness, max_fitness, avg_fitness;
    min_fitness = std::numeric_limits<double>::max();
    max_fitness = -std::numeric_limits<double>::max();
    avg_fitness = 0;

    for (int j = 0; j < population.size(); j++) {
        population[j]->get_edges(n_input_nodes, n_output_nodes, n_hidden_layers, nodes_per_layer, all_nodes, all_nodes_by_layer, edges, recurrent_edges);
        population[j]->fitness = objective_function(n_hidden_layers, nodes_per_layer, edges, recurrent_edges);

        if (min_fitness > population[j]->fitness) min_fitness = population[j]->fitness;
        if (max_fitness < population[j]->fitness) max_fitness = population[j]->fitness;
        avg_fitness += population[j]->fitness;
    }
    avg_fitness /= population.size();

    for (int i = 0; i < max_iterations; i++) {
        cout << "ITERATION " << i << " -- min: " << min_fitness << ", avg: " << avg_fitness << ", max: " << max_fitness << endl;

        /*
        for (int j = 0; j < population.size(); j++) {
            cout << population[j] << endl;
        }
        */
        cout << "\t[innovations:" << innovations.size() << "]" << endl;
        /*
        for (int i = 0; i < innovations.size(); i++) {
            cout << "\t" << i << " - " << innovations[i] << endl;
        }
        */

        assign_species();
        cout << "\tassigned species, n_species: " << species.size() << ", sizes:";
        for (int j = 0; j < species.size(); j++) {
            cout << " " << species[j].size() << " (n_genes: " << species[j][0]->connection_genes.size() << ")";
        }
        cout << endl;

        next_population();

        min_fitness = std::numeric_limits<double>::max();
        max_fitness = -std::numeric_limits<double>::max();
        avg_fitness = 0;
        for (int j = 0; j < population.size(); j++) {
            population[j]->get_edges(n_input_nodes, n_output_nodes, n_hidden_layers, nodes_per_layer, all_nodes, all_nodes_by_layer, edges, recurrent_edges);
            population[j]->fitness = objective_function(n_hidden_layers, nodes_per_layer, edges, recurrent_edges);

            if (min_fitness > population[j]->fitness) min_fitness = population[j]->fitness;
            if (max_fitness < population[j]->fitness) max_fitness = population[j]->fitness;
            avg_fitness += population[j]->fitness;
        }
        avg_fitness /= population.size();
    }
}

