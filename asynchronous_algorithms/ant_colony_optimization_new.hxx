#ifndef TAO_ACO_NEW_H
#define TAO_ACO_NEW_H

#include <vector>
using std::vector;

#include "./neural_networks/edge_new.hxx"

class AntColonyPaths {
    public:
        double fitness;

        vector<EdgeNew> edges;
        vector<EdgeNew> recurrent_edges;

        AntColonyPaths(double _fitness, const vector<EdgeNew> &edges, const vector<EdgeNew> &recurrent_edges);

        friend bool operator<(const AntColonyPaths &e1, const AntColonyPaths &e2);
};

class AntColonyNode {
    private:
        uint32_t depth;
        uint32_t layer;
        uint32_t node;

        double recurrent_pheromone;
        AntColonyNode *recurrent_target;

        vector<double> forward_target_pheromones;
        vector<AntColonyNode*> forward_targets;

    public:
        AntColonyNode(uint32_t _depth, uint32_t _layer, uint32_t _node);

        uint32_t get_depth() const;
        uint32_t get_layer() const;
        uint32_t get_node() const;

        void connect(AntColonyNode *target);

        uint32_t get_number_connections();
        void get_connection(uint32_t connection, uint32_t &depth, uint32_t &layer, uint32_t &node);
        double get_pheromone(uint32_t connection);

        void increment_pheromones(uint32_t dst_depth, uint32_t dst_layer, uint32_t dst_node, double pheromone_placement_rate, double pheromone_maximum);
        void decrement_pheromones(uint32_t dst_depth, uint32_t dst_layer, uint32_t dst_node, double pheromone_degradation_rate, double pheromone_minimum);
        //void decrement_pheromones(double pheromone_degradation_rate, double minimum_pheromone);

        AntColonyNode* select_path();

        string print_pheromones();

        friend ostream& operator<< (ostream& out, const AntColonyNode &aco_node);
        friend ostream& operator<< (ostream& out, const AntColonyNode *aco_node);
};

class AntColonyNew {
    private:
        double pheromone_placement_rate;
        double pheromone_degradation_rate;

        double maximum_pheromone;
        double minimum_pheromone;

        uint32_t number_ants;

        uint32_t recurrent_depth;
        uint32_t n_input_nodes;
        uint32_t n_hidden_layers;
        uint32_t n_hidden_nodes;
        uint32_t n_output_nodes;

        uint32_t max_population_size;
        uint32_t iteration;

        string output_directory;

        AntColonyNode* start_node;
        vector< vector< vector<AntColonyNode*> > > nodes;

        vector< AntColonyPaths* > population;
        vector<string> input_labels;
        vector<string> output_labels;


        bool node_exists(uint32_t depth, uint32_t layer, uint32_t node);

    public:

        AntColonyNew(double _pheromone_placement_rate, double _pheromone_degradation_rate, double _maximum_pheromone, double _minimum_pheromone, uint32_t _number_ants, uint32_t _recurrent_depth, uint32_t _n_input_nodes, uint32_t _n_hidden_layers, uint32_t _n_hidden_nodes, uint32_t _n_output_nodes, uint32_t max_population_size);

        ~AntColonyNew();

        //void decrement_pheromones();
        void decrement_pheromones(const vector<EdgeNew> &edges, const vector<EdgeNew> &recurrent_edges);
        void increment_pheromones(const vector<EdgeNew> &edges, const vector<EdgeNew> &recurrent_edges);
        void add_ant_paths(double fitness, const vector<EdgeNew> &edges, const vector<EdgeNew> &recurrent_edges);

        void generate_neural_network(vector<EdgeNew> &edges, vector<EdgeNew> &recurrent_edges);
        void generate_fully_connected_neural_network(vector<EdgeNew> &edges, vector<EdgeNew> &recurrent_edges);

        void print_pheromones();

        double get_best_fitness();
        double get_avg_fitness();
        double get_worst_fitness();

        uint32_t get_iteration();

        void set_labels(const vector<string> &_input_labels, const vector<string> &_output_labels);
        void set_output_directory(string od);
        void write_population();
};

#endif
