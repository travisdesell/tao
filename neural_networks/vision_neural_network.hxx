#ifndef TAO_VISION_NEURAL_NETWORK_H
#define TAO_VISION_NEURAL_NETWORK_H

#include "edge.hxx"

//OPENCV INCLUDES
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"

using namespace cv;

#include <vector>

class VisionNeuralNetwork {
    private:
        int image_size;
        std::vector<Mat> images;
        std::vector<int> classifications;


        int n_layers;
        int n_hidden_layers;
        int nodes_per_layer;
        double **nodes;

        std::vector<Edge> edges;

    public:
        VisionNeuralNetwork(const std::vector<Mat> &_images, const vector<int> &classifications, int _n_hidden_layers, int _nodes_per_layer);

        void initialize_nodes(int _n_hidden_layers, int _nodes_per_layer);

        int get_n_edges();
        void set_edges(const std::vector<Edge> &e);

        void reset();

        double objective_function(const std::vector<double> &parameters);
        double objective_function();

        double evaluate(Mat &image, int classification);
        double evaluate();
};


#endif
