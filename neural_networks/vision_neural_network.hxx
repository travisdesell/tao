#ifndef TAO_VISION_NEURAL_NETWORK_H
#define TAO_VISION_NEURAL_NETWORK_H

#include "edge.hxx"

#include <vector>
using std::vector;

class VisionNeuralNetwork {
    private:
        int image_size;
        vector<char*> images;
        vector<int> classifications;


        int n_layers;
        int n_hidden_layers;
        int nodes_per_layer;
        double **nodes;

        vector<Edge> edges;

    public:
        VisionNeuralNetwork(int _image_size, const vector<char*> &_images, const vector<int> &classifications, int _n_hidden_layers, int _nodes_per_layer);

        void initialize_nodes(int _n_hidden_layers, int _nodes_per_layer);

        int get_n_edges();
        void set_edges(const vector<Edge> &e);

        void reset();

        double objective_function(const vector<double> &parameters);
        double objective_function();

        double evaluate(const char *image, int classification);
        double evaluate();
};


#endif
