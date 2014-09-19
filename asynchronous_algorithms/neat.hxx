
#include <vector>
using std::vector;


double get_random_weight() {
    return (drand48() * 2.0) - 1.0;
}

void get_random_unique_pair(vector<int> &node_genes, int &input, int &output) {
    input = drand48() * node_genes.size();
    output = drand48() * (node_genes.size() - 1);
    if (output == input) output++;

    input = node_genes[input];  //get the input^th gene
    output = node_genes[output]; //get the output^th gene
}


class NEATGene {
    public:
        bool enabled;
        double weight;
        int input_node;
        int output_node;
        int innovation;

        NEATGene(bool en, double we, int in, int oo, int inn) : enabled(en), weight(we), input_node(in), output_node(on), innovation(inn) {
        }

};

class NEATIndividual {
    public:
        double fitness;
        vector<int> node_genes;
        vector< NEATGene* > connection_genes;

}


class NEAT {
    protected:
        int last_innovation;
        double excess_weight;
        double disjoint_weight;
        double weight_weight;
        double compatibility_threshold;

        int population_size;
        vector< NEATIndividual* > population;

        vector<int> nodes;
        vector< NEATGene* > innovations;

        vector< vector<NEATIndividual*> > species;

    public:
        NEAT(double ew, double dw, double ww, double ct, int ps) : last_innovation(0), excess_weight(ew), disjoint_weight(dw), weight_weight(ww), compatability_threshold(ct), population_size(ps) {
        }

        NEATIndividual* mutate_add_link(NEATIndividual *parent) {
            NEATIndividul *result = new NEATIndividual(parent);

            int input, output;
            get_random_unique_pair(node_genes, input, output);
            NEATGene *new_connection = new NEATGene(true, get_random_weight(), input, output, innovations);

            result->add_connection_gene(new_connection);
            return result;
        }

        NEATIndividual* mutate_add_node(NEATIndividual *parent) {
            parent->enabled = false;
            //add a new node in between the nodes of the conncetion
            //disable the parent
            //first child gets a weight of 1.0, second gets the parent's weight
            NEATIndividual *result = new NEATIndividual(parent);

            NEATGene *connection_to_split = result.connection_genes[ result.connection_genes.size() * drand48() ];
            int new_node = node_genes.back() + 1;
            NEATGene *first = new NEATGene(true, 1.0, connection_to_split->input_node, new_node, innovations);
            NEATGene *second = new NEATGene(true, connection_to_split->weight, new_node, connection_to_split->output_node, innovations);

            result->add_connction_gene(first);
            result->add_connction_gene(second);
            return result;
        }

        NEATIndividual* mutate_weights(NEATIndividual *parent) {
            for (int i = 0; i < parent->connection_genes.size(); i++) {
                double randval = drand48();
                if (randval < random_weight_mutation_rate) {
                    parent->connection_genes[i]->weight = get_random_weight();
                } else if (randval < random_weight_mutation_rate + uniform_weight_mutation_rate) {
                    double randval2 = drand48();
                    if (randval2 < 0.5) {
                        parent->connection_genes[i]->weight += uniform_perturbation;
                    } else {
                        parent->connection_genes[i]->weight -= uniform_perturbation;
                    }
                }
            }
        }

        NEATIndividual* crossover(NEATIndividual *p1, NEATIndividual *p2) {
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
                        j++;

                    } else if (p2->fitness > p1->fitness) {
                        //p2 is more fit and gene is excess, add it
                        new_individual->add_connection_gene( p2->connection_genes[j] );
                        j++;
                    }

                } else if (j >= p2->connection_genes.size()) {
                    if (p1->fitness == p2->fitness) {
                        //p2's gene is excess, fitnesses are equal so add randomly
                        if (drand48() < 0.5) {
                            new_individual->add_connection_gene( p1->connection_genes[i] );
                        }
                        i++;

                    } else if (p1->fitness > p2->fitness) {
                        //p2 is more fit and gene is excess, add it
                        new_individual->add_connection_gene( p1->connection_genes[i] );
                        i++;
                    }

                } else {
                    if (p1->connection_genes[i]->innovation == p2->connection_genes[j]->innovation) {
                        //same gene, pick one randomly
                        if (drand48() < 0.5) {
                            new_individual->add_connection_gene( p1->connection_genes[i] );
                        } else {
                            new_individual->add_connection_gene( p1->connection_genes[j] );
                        }
                        i++;
                        j++;

                    } else if (p1->connection_genes[i] < p2->connection_genes[j]->innovation) {
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
        }

        void assign_species() {
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
                    vector<NEATIndividaul*> new_species;
                    new_species.push_back(population[i]);
                    species.push_back(new_species);

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
                        while (ri < representative->connection_genes.size() && pi < population[i]->connection_genes.size()) {
                            if (ri >= representative->connection_genes.size()) {
                                excess_count++;
                                ri++;
                                continue;
                            }

                            if (pi >= population[i]->connection_genes.size()) {
                                excess_count++;
                                pi++;
                                continue;
                            }

                            int rep_innovation = representative->connection_genes[ri].innovation;
                            int pop_innovation = population[i]->connection_genes[pi].innovation;

                            if (rep_innovation == pop_innovation) {
                                avg_weight_count++;
                                avg_weight_difference += fabs(representative->connection_genes[ri].weight - population[i]->connection_genes[pi].weight);
                            } else if (rep_innovation < pop_innovation) {
                                ri++;
                                disjoint_count++;
                            } else {
                                pi++;
                                disjoint_count++;
                            }
                        }

                        avg_weight_difference /= avg_weight_count;

                        double compatability = (c1 * excess_count) / normalization;
                        compatability += (c2 * disjoint_count) / normalization;
                        compatability += c3 * avg_weight_difference;

                        if (compatability < compatability_threshold) {
                            species[j].push_back(population[i]);
                            matches_species = true;
                            break;
                        }
                    }

                    if (!matches_species) {
                        vector<NEATIndividaul*> new_species;
                        new_species.push_back(population[i]);
                        species.push_back(new_species);
                    }
                }
            }
        }

        NEATIndividual *interspecies_crossover() {
            int first, second;
            first = population.size() * drand48();
            second = (population.size() - 1) * drand48();
            if (second == first) second++;

            NEATIndividual *f, *s;
            f = population[first];
            s = population[second];
            return crossover(f, s);
        }

        NEATIndividual *intraspecies_crossover() {
            //select a species at random


            //select two individuals within the species
            int first, second;
            first = species.size() * drand48();
            second = (species.size() - 1) * drand48();
            if (second == first) second++;

            NEATIndividual *f, *s;
            f = species[first];
            s = species[second];
            return crossover(f, s);
        }

        bool is_champion(int individual) {
        }

        void initialize_population(int n_input_nodes, int n_output_nodes) {
            for (int k = 0; k < population_size; k++) {
                NEATIndividual *current = new NEATIndividual();
                for (int i = 0; i < n_input_nodes; i++) {
                    for (int j = 0; j < n_output_nodes; j++) {
                        current->add_connection_gene(new NEATGene(true, get_random_weight(), i, j, innovations));
                    }
                }
                population.push_back(current);
            }
        }

        /**
         *  settings:
         *      species stagnation number of iterations (to determine if species members can reproduce)
         *      mutation without crossover rate
         *      weight mutation rate
         *      add node mutation rate
         *      add linke mutation rate
         *      interspecies crossover rate
         */

        void next_population() {
            //options:
            //  crossover -- remaining?
            //  mutation: 25% mutation without crossover
            //      -- 80% chance of genome having its values perturbed, 90% uniform change, 10% random new value per gene
            //  selection:
            //      -- champion of each species with more than 5 networks copied

            vector< NEATIndividual* > next_population;

            for (int k = 0; k < population_size; k++) {
                if (is_champion(i)) {
                    next_population.push_back(population[i].copy());
                }
            }

            while (next_population.size() < population_size) {
                double randval = drand48();
                if (randval < mutation_without_crossover_rate) {
                    NEATIndividual *parent = get_random_parent();

                    randval = drand48();
                    if (randval < weight_mutation_rate) {
                        next_population.push_back( mutate_weights(parent) );
                    }

                    randval = drand48();
                    if (randval < add_node_mutation_rate) {
                        next_population.push_back( mutate_add_node(parent) );
                    }

                    randval = drand48();
                    if (randval < add_link_mutation_rate) {
                        next_population.push_back( mutate_add_link(parent) );
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

        void iterate(int max_iterations, int n_input_nodes, int n_output_nodes, double (*objective_function)(const vector<Edge> &edges, const vector<Edge> &recurrent_edges)) {
            vector<Edge> edges;
            vector<Edge> recurrent_edges;

            initialize_population(int n_input_nodes, int n_output_nodes);
            for (int j = 0; j < population.size(); j++) {
                population[j].get_edges(edges, recurrent_edges);
                population[j].fitness = objective_function(edges, recurrent_edges);
            }

            for (int i = 0; i < max_iterations; i++) {
                assign_species();
                next_population();

                for (int j = 0; j < population.size(); j++) {
                    population[j].get_edges(edges, recurrent_edges);
                    population[j].fitness = objective_function(edges, recurrent_edges);
                }
            }
        }
};
