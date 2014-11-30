#ifndef TAO_CONVOLUTIONAL_NEURAL_NETWORK_H
#define TAO_CONVOLUTIONAL_NEURAL_NETWORK_H

#include <utility>
using std::pair;

#include <vector>
using std::vector;

class ConvolutionalNeuralNetwork {
    private:
        int image_x, image_y;
        bool rgb;

        vector< vector< vector<char> > > images;

        vector< pair<int, int> > layers;

        int fc_size;
        int n_classes;

        int total_weights;
        vector< vector< vector<double> > > nodes;

        vector<double> weights;

    public:
        ConvolutionalNeuralNetwork(int _image_x, int _image_y, bool _rgb, const vector< vector< vector<char> > > &_images, const vector< pair<int, int> > &_layers, int _fc_size);


        void initialize_nodes(const vector< pair<int, int> > &_layers, int _fc_size);
 
        int get_n_edges();

        void reset();

        double objective_function(const vector<double> &parameters);
        double objective_function();

        void print_statistics(const vector<double> &parameters);

        double evaluate(const vector<char> &image, int classification);
        double evaluate();
};


#endif
