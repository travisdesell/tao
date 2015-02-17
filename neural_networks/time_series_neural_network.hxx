#ifndef TAO_TIME_SERIES_NEURAL_NETWORK_H
#define TAO_TIME_SERIES_NEURAL_NETWORK_H

#include <vector>

#include "edge.hxx"

using std::vector;

/**
 *  0               -   input later
 *  1 ... (N*2)+1   -   hidden/recurrent layers (odd layers are recurrent layers, even layers are hidden layers)
 *  1               -   recurrent layer for input layer
 *  2               -   1st hidden layer
 *  3               -   recurrent layer for 1st hidden layer
 *  4               -   2nd hidden layer
 *  5               -   recurrent layer for 2nd hidden layer
 *  ...
 *  (N*2)+2         -   output layer
 */

class TimeSeriesNeuralNetwork {
    private:
        int time_series_rows;
        int time_series_columns;
        int target_parameter;
        double **time_series_data;

        int n_layers;
        int n_hidden_layers;
        int nodes_per_layer;
        double **nodes;

        vector<Edge> edges;
        vector<Edge> recurrent_edges;

    public:
        TimeSeriesNeuralNetwork(int target_parameter);

        TimeSeriesNeuralNetwork(double **tsd, int tsr, int tsc, int tp, int nhl, int npl, vector<Edge> e, vector<Edge> re);

        ~TimeSeriesNeuralNetwork();

        int get_n_edges();

        void reset();
        void reset_non_recurrent();
        double evaluate();
        double objective_function();
        double objective_function(const vector<double> &parameters);

        void set_time_series_data(double **tsd, int tsr, int tsc);
        void initialize_nodes(int nhl, int npl);
        void set_edges(const vector<Edge> &e, const vector<Edge> &re);
        //void read_weights_from_file(string weights_filename);
        void read_nn_from_file(string nn_filename);
};


#endif
