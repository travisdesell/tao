#include <cmath>

#include <fstream>
using std::ifstream;

#include <iostream>
using std::cerr;
using std::endl;

#include <string>
using std::string;

#include <vector>
using std::vector;

#include <boost/tokenizer.hpp>
using boost::tokenizer;
using boost::char_separator;

#include "time_series_neural_network.hxx"


TimeSeriesNeuralNetwork::TimeSeriesNeuralNetwork(int tp) : target_parameter(tp) {
}


TimeSeriesNeuralNetwork::TimeSeriesNeuralNetwork(double **tsd, int tsr, int tsc, int tp, int nhl, int npl, vector<Edge> e, vector<Edge> re) : target_parameter(tp), edges(e), recurrent_edges(re) {

    set_time_series_data(tsd, tsr, tsc);
    initialize_nodes(nhl, npl);
}

int TimeSeriesNeuralNetwork::get_n_edges() {
    return edges.size();
}

void TimeSeriesNeuralNetwork::initialize_nodes(int nhl, int npl) {
    n_hidden_layers = nhl;
    nodes_per_layer = npl;
    n_layers = 1 + (n_hidden_layers * 2) + 1;

    //allocate and initailize the nodes
    nodes = new double*[n_layers];

    //allocate and initialze the input layer
    nodes[0] = new double[time_series_columns + 1]; //input nodes are == the number of time series columns + 1 for bias
    for (int j = 0; j < time_series_columns; j++) {
        nodes[0][j] = 0.0;
    }
    nodes[0][time_series_columns] = 1.0;    //bias node

    //allocate and initialize the hidden layer nodes
    for (int i = 1; i < (n_layers - 1); i++) {
        nodes[i] = new double[nodes_per_layer + 1]; // +1 for bias
        for (int j = 0; j < nodes_per_layer; j++) {
            nodes[i][j] = 0.0;
        }
        nodes[i][nodes_per_layer] = 1.0;    //bias node
    }

    //allocate and initialize the output layer
    nodes[n_layers - 1] = new double[1];
    nodes[n_layers - 1][0] = 0.0;
}


void TimeSeriesNeuralNetwork::set_time_series_data(double **tsd, int tsr, int tsc) {
    time_series_rows = tsr;
    time_series_columns = tsc;

    //allocate and initialize the time series data by copying over 
    //the passed time series data.
    time_series_data = new double*[time_series_rows];
    for (int i = 0; i < time_series_rows; i++) {
        time_series_data[i] = new double[time_series_columns];
        for (int j = 0; j < time_series_columns; j++) {
            time_series_data[i][j] = tsd[i][j];
        }
    }
}

TimeSeriesNeuralNetwork::~TimeSeriesNeuralNetwork() {
    for (int i = 0; i < time_series_rows; i++) {
        delete[] time_series_data[i];
    }
    delete[] time_series_data;

    for (int i = 0; i < n_layers; i++) {
        delete[] nodes[i];
    }
    delete[] nodes;

}

void TimeSeriesNeuralNetwork::reset() {
    for (int j = 0; j < time_series_columns; j++) {
        nodes[0][j] = 0.0;
    }

    for (int i = 1; i < n_layers - 1; i++) {
        for (int j = 0; j < nodes_per_layer; j++) {
            nodes[i][j] = 0.0;
        }
    }

    nodes[n_layers - 1][0] = 0.0;
}

void TimeSeriesNeuralNetwork::reset_non_recurrent() {
    for (int j = 0; j < time_series_columns; j++) {
        nodes[0][j] = 0.0;
    }

    for (int i = 2; i < n_layers - 1; i += 2) {
        for (int j = 0; j < nodes_per_layer; j++) {
            nodes[i][j] = 0.0;
        }
    }

    nodes[n_layers - 1][0] = 0.0;
}

double TimeSeriesNeuralNetwork::evaluate() {
    double mean_average_error = 0.0;

    for (int ts_row = 0; ts_row < (time_series_rows - 1); ts_row++) {
        //reset the non-recurrent nodes
        reset_non_recurrent();

        //set the input nodes to the time series data input
        for (int i = 0; i < time_series_columns; i++) {
            nodes[0][i] = time_series_data[ts_row][i];
        }

        //calculate the neural network to the output node
        for (int i = 0; i < edges.size(); i++) {
            Edge e = edges.at(i);
            nodes[e.dst_layer][e.dst_node] += nodes[e.src_layer][e.src_node] * e.weight;

//            cerr << e << " : node: " << nodes[e.dst_layer][e.dst_node] << endl;
        }

        for (int i = 0; i < recurrent_edges.size(); i++) {
            Edge e = recurrent_edges.at(i);
            nodes[e.dst_layer][e.dst_node] = nodes[e.src_layer][e.src_node];
        }

        //update the mean average error
        //might need to deal with summation errors here
        //might want to add activation function here
        mean_average_error += fabs(nodes[n_layers - 1][0] - time_series_data[ts_row + 1][target_parameter]);
    }

    mean_average_error /= (time_series_rows - 1);

    return -mean_average_error;
}

double TimeSeriesNeuralNetwork::objective_function(const vector<double> &parameters) {
    reset();

    for (int i = 0; i < parameters.size(); i++) {
        edges[i].weight = parameters[i];
    }

    return evaluate();
}

/**
 *  weights filename is one weight per line
 */
void TimeSeriesNeuralNetwork::read_weights_from_file(string weights_filename) {
    ifstream weights_file( weights_filename.c_str() );
    if (!weights_file.is_open()) {
        cerr << "Error, could not open file: '" << weights_filename << "' for reading." << endl;
        exit(1);
    }

    int i = 0;
    string s;
    getline( weights_file, s );
    while (weights_file.good()) {

        edges.at(i).weight = atoi(s.c_str());
        getline( weights_file, s );
    }
}

void TimeSeriesNeuralNetwork::set_edges(const vector<Edge> &e, const vector<Edge> &re) {

    edges.clear();
    edges = e;

    recurrent_edges.clear();
    recurrent_edges = re;
}

void TimeSeriesNeuralNetwork::read_nn_from_file(string nn_filename) {
    ifstream nn_file( nn_filename.c_str() );
    if (!nn_file.is_open()) {
        cerr << "Error, could not open file: '" << nn_filename << "' for reading." << endl;
        exit(1);
    }
    string s;

    //read n_hidden_layers
    getline( nn_file, s );
    while (nn_file.good() && 0 != s.compare("#hidden layers")) getline( nn_file, s );
    getline( nn_file, s );
    int nhl = atoi(s.c_str());

    //read nodes_per_layer
    getline( nn_file, s );
    while (nn_file.good() && 0 != s.compare("#nodes per layer")) getline( nn_file, s );
    getline( nn_file, s );    
    int npl = atoi(s.c_str());

    initialize_nodes(nhl, npl);

    //read edges
    edges.clear();

    getline( nn_file, s );
    while (nn_file.good() && 0 != s.compare("#feed forward edges")) getline( nn_file, s );

    getline( nn_file, s );
    while (nn_file.good() && 0 != s.compare("")) {

        char_separator<char> sep(" ", "");
        tokenizer<char_separator<char> > tok(s, sep);

        tokenizer< char_separator<char> >::iterator i = tok.begin(); 

        int src_layer = atoi((*i).c_str());
        int dst_layer = atoi((*(++i)).c_str());
        int src_node  = atoi((*(++i)).c_str());
        int dst_node  = atoi((*(++i)).c_str());

        edges.push_back(Edge(src_layer, dst_layer, src_node, dst_node));
        getline( nn_file, s );
    }

    getline( nn_file, s );

    while (nn_file.good() && 0 != s.compare("#recurrent edges")) {
        getline( nn_file, s );
    }

    getline( nn_file, s );
    while (nn_file.good() && 0 != s.compare("")) {

        char_separator<char> sep(" ", "");
        tokenizer<char_separator<char> > tok(s, sep);

        tokenizer< char_separator<char> >::iterator i = tok.begin(); 

        int src_layer = atoi((*i).c_str());
        int dst_layer = atoi((*(++i)).c_str());
        int src_node  = atoi((*(++i)).c_str());
        int dst_node  = atoi((*(++i)).c_str());

        recurrent_edges.push_back(Edge(src_layer, dst_layer, src_node, dst_node));
        getline( nn_file, s );
    }
}
