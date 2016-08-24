#ifndef TAO_NEURAL_NETWORK_H
#define TAO_NEURAL_NETWORK_H

#include <fstream>
using std::ofstream;

#include <string>
using std::string;

#include <vector>
using std::vector;

#include "./neural_networks/edge_new.hxx"

#include "./util/tao_random.hxx"

typedef double (*ActivationFunction)(double);
typedef double (*DerivativeFunction)(double);

double linear_activation_function(double input);
double relu_activation_function(double input);
double sigmoid_activation_function(double input);
double tanh_activation_function(double input);

double linear_derivative(double input);
double relu_derivative(double input);
double sigmoid_derivative(double input);
double tanh_derivative(double input);

class Neuron {
    public:
        uint32_t depth;
        uint32_t layer;
        uint32_t node;

        string identifier;
        double bias;
        double value;
        double error;

        vector<EdgeNew*> forward_edges;
        vector<EdgeNew*> backward_edges;

        Neuron(uint32_t depth, uint32_t layer, uint32_t node);
        ~Neuron();

        void connect_forward(EdgeNew *edge);
        void connect_backward(EdgeNew *edge);

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

        uint32_t output_layer;
        uint32_t active_input_nodes;

        vector< vector< vector<Neuron*> > > nodes;
        vector<Neuron*> linear_nodes;

        vector<EdgeNew*> edges;
        vector<EdgeNew*> recurrent_edges;

        vector< vector<double> > inputs;
        vector< vector<double> > outputs;

        bool kahan_summation, batch_update;

        string activation_function_str;
        ActivationFunction activation_function;
        DerivativeFunction derivative_function;

    public:
        void initialize_nodes();
        void set_activation_functions(string activation_function_str);

        NeuralNetwork(string json_filename);

        NeuralNetwork(uint32_t _recurrent_depth, uint32_t _n_input_nodes, uint32_t _n_hidden_layers, uint32_t _n_hidden_nodes, uint32_t _n_output_nodes, string _activation_function_str);

        ~NeuralNetwork();
        void reset();

        void set_training_data(uint32_t n_examples, uint32_t input_size, double **_inputs, uint32_t output_size, double **_outputs);
        void set_training_data(const vector< vector<double> > &_inputs, const vector< vector<double> > &_outputs);

        void set_edges(const vector<EdgeNew> &_edges, const vector<EdgeNew> &_recurrent_edges);
        void set_edges(const vector<EdgeNew> &_edges, const vector<EdgeNew> &_recurrent_edges, const vector<string> &input_labels, const vector<string> &output_labels);

        string json();
        void write_to_file(string json_filename);
        void print_predictions(ofstream &outfile);

        uint32_t get_parameter_size();

        void set_weights(const vector<double> &weights);
        void evaluate_at(uint32_t example);
        void evaluate_at_softmax(uint32_t example);

        void update_recurrent_nodes();
        double objective_function(const vector<double> &weights);
        double backpropagation_time_series(const vector<double> &starting_weights, double learning_rate, uint32_t max_iterations);
        double backpropagation_stochastic(const vector<double> &starting_weights, double learning_rate, uint32_t max_iterations, TaoRandom &generator);

        void get_gradient(vector<double> &gradient);
        void get_gradient_stochastic(vector<double> &gradient);

        void use_kahan_summation(bool val);
        void use_batch_update(bool val);

        bool read_checkpoint(string checkpoint_filename, TaoRandom &generator, vector<uint32_t> &shuffled_examples, uint32_t &iteration);
        void write_checkpoint(string checkpoint_filename, TaoRandom &generator, const vector<uint32_t> &shuffled_examples, uint32_t iteration);

        friend ostream& operator<< (ostream& out, const NeuralNetwork &neural_network);
        friend ostream& operator<< (ostream& out, const NeuralNetwork *neural_network);
};


#endif
