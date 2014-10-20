#ifndef TAO_NEAT_H
#define TAO_NEAT_H

#include <set>
using std::set;

#include <vector>
using std::vector;

#include "neural_networks/edge.hxx"



class NEATGene {
    public:
        bool enabled;
        double weight;
        int input_node;
        int output_node;
        int innovation;

        NEATGene(bool en, double we, int in, int on, int inn);
        NEATGene(bool en, double we, int in, int on, vector<NEATGene*> &innovations);
        NEATGene(NEATGene *parent);

        friend ostream& operator<< (ostream& out, const NEATGene *gene);
};

ostream& operator<< (ostream& out, const NEATGene *gene);

class NEATNode {
    public:
        int id;

        int layer;
        int node;

        NEATNode(int i, int l, int n);
};

class NEATIndividual {
    public:
        double fitness;
        int species;
        int champion_iterations;
        vector<int> node_genes;
        vector< NEATGene* > connection_genes;

        void add_connection_gene(NEATGene* gene);
        void set_node_genes();

        NEATIndividual();
        NEATIndividual(NEATIndividual *parent);

        void get_edges(int n_input_nodes, int n_output_nodes, int &n_hidden_layers, int &nodes_per_layer, const vector<NEATNode*> &all_nodes, const vector< vector<NEATNode*> > &all_nodes_by_layer, vector<Edge> &edges, vector<Edge> &recurrent_edges);

        friend ostream& operator<< (ostream& out, const NEATIndividual *edge);
};

ostream& operator<< (ostream& out, const NEATIndividual *individual);

class NEAT {
    protected:
        int last_innovation;
        double excess_weight;
        double disjoint_weight;
        double weight_weight;
        double compatibility_threshold;
        double normalization;

        double mutation_without_crossover_rate; 
        double weight_mutation_rate; 
        double add_node_mutation_rate; 
        double add_link_mutation_rate; 
        double interspecies_crossover_rate;
        double crossover_weight_average_rate;

        double random_weight_mutation_rate;
        double uniform_weight_mutation_rate;
        double uniform_perturbation;

        double enable_if_both_parents_disabled;

        int population_size;
        vector< NEATIndividual* > population;

        vector< NEATGene* > innovations;

        vector< NEATNode* > all_nodes;
        vector< vector<NEATNode*> > all_nodes_by_layer;

        vector< vector<NEATIndividual*> > species;

    public:
        /**
         *  settings:
         *      species stagnation number of iterations (to determine if species members can reproduce)
         *      mutation without crossover rate
         *      weight mutation rate
         *      add node mutation rate
         *      add linke mutation rate
         *      interspecies crossover rate
         *
         *  options:
         *      crossover -- remaining?
         *      mutation: 25% mutation without crossover
         *          -- 80% chance of genome having its values perturbed, 90% uniform change, 10% random new value per gene
         *      selection:
         *          -- champion of each species with more than 5 networks copied
         */

        NEAT(double ew, double dw, double ww, double ct, double norm, double mwcr, double wmr, double anmr, double almr, double icr, double cwar, double rwmr, double uwmr, double up, double eibpd, int ps);

        NEATIndividual* mutate_add_link(NEATIndividual *parent);
        NEATIndividual* mutate_add_node(NEATIndividual *parent);
        NEATIndividual* mutate_weights(NEATIndividual *parent);
        NEATIndividual* crossover(NEATIndividual *p1, NEATIndividual *p2);

        void assign_species();
        bool is_champion(NEATIndividual *individual);

        NEATIndividual *interspecies_crossover();
        NEATIndividual *intraspecies_crossover();

        void initialize_population(int n_input_nodes, int n_output_nodes);
        void next_population();

        void iterate(int max_iterations, int n_input_nodes, int n_output_nodes, double (*objective_function)(int n_hidden_layers, int nodes_per_layer, const vector<Edge> &edges, const vector<Edge> &recurrent_edges));
};

#endif
