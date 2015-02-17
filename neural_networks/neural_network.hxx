#ifndef TAO_NEURAL_NETWORK_H
#define TAO_NEURAL_NETWORK_H

#include <string>
using std::string;

#include <vector>
using std::vector;

#include "./neural_networks/edge_new.hxx"


class Neuron {
    public:
        uint32_t depth;
        uint32_t layer;
        uint32_t node;

        string identifier;
        double bias;
        double value;

        vector<EdgeNew> forward_edges;
        vector<EdgeNew> backward_edges;

        Neuron(uint32_t depth, uint32_t layer, uint32_t node);

        void connect_forward(EdgeNew edge);
        void connect_backward(EdgeNew edge);

        string json() const;

        friend ostream& operator<< (ostream& out, const Neuron &neuron);
        friend ostream& operator<< (ostream& out, const Neuron *neuron);
};


class NeuralNetwork {
    private:
        uint32_t recurrent_depth;
        uint32_t n_input_nodes;
        uint32_t n_hidden_layers;
        uint32_t n_hidden_nodes;
        uint32_t n_output_nodes;

        vector< vector< vector<Neuron*> > > nodes;

        vector<EdgeNew> edges;
        vector<EdgeNew> recurrent_edges;

        vector< vector<double> > inputs;
        vector< vector<double> > outputs;

    public:
        NeuralNetwork(string json_filename);

        NeuralNetwork(uint32_t _recurrent_depth, uint32_t _n_input_nodes, uint32_t _n_hidden_layers, uint32_t _n_hidden_nodes, uint32_t _n_output_nodes);

        void set_training_data(uint32_t n_examples, uint32_t input_size, const double **_inputs, uint32_t output_size, double **_outputs);
        void set_training_data(const vector< vector<double> > &_inputs, const vector< vector<double> > &_outputs);

        void set_edges(const vector<EdgeNew> &_edges, const vector<EdgeNew> &_recurrent_edges);
        void set_edges(const vector<EdgeNew> &_edges, const vector<EdgeNew> &_recurrent_edges, const vector<string> &input_labels, const vector<string> &output_labels);

        string json();
        void write_to_file(string json_filename);

        void get_gradient(vector<double> &gradient);
        void get_gradient_stochastic(vector<double> &gradient);

        friend ostream& operator<< (ostream& out, const NeuralNetwork &neural_network);
        friend ostream& operator<< (ostream& out, const NeuralNetwork *neural_network);
};


#endif
