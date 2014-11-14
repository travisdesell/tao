#include "edge.hxx"
#include "vision_neural_network.hxx"

//OPENCV INCLUDES
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"

#include <iostream>
using std::cerr;
using std::endl;

#include <vector>
using namespace cv;


VisionNeuralNetwork::VisionNeuralNetwork(const std::vector<Mat> &_images, const vector<int> &_classifications, int _n_hidden_layers, int _nodes_per_layer) : images(_images), classifications(_classifications) {

    image_size = images[0].rows * images[0].cols * 3;

    nodes = NULL;
    initialize_nodes(n_hidden_layers, nodes_per_layer);
}

void VisionNeuralNetwork::initialize_nodes(int _n_hidden_layers, int _nodes_per_layer) {
    if (nodes != NULL) {
        //if we're reinitializing, we need to delete the previously instantiated
        //nodes.
        for (int i = 0; i < n_layers; i++) {
            delete[] nodes[i];
        }
        delete[] nodes;
    }

    n_hidden_layers = _n_hidden_layers;
    nodes_per_layer = _nodes_per_layer + 1; //add an additional bias node
    n_layers = n_hidden_layers + 2;

    nodes = new double*[n_layers];
    nodes[0] = new double[image_size + 1];  //add a bias node

    for (int i = 1; i < n_layers - 1; i++) {
        nodes[i] = new double[nodes_per_layer];
    }

    nodes[n_layers - 1] = new double[1];
    reset();

}

void VisionNeuralNetwork::reset() {
    for (int j = 0; j < image_size; j++) {
        nodes[0][j] = 0.0;
    }
    nodes[0][image_size] = 1.0; //bias node

    for (int i = 1; i < n_layers - 1; i++) {
        for (int j = 0; j < nodes_per_layer - 1; j++) {
            nodes[i][j] = 0.0;
        }
        nodes[i][nodes_per_layer - 1] = 1.0; //bias node
    }

    nodes[n_layers - 1][0] = 0.0;
}

double VisionNeuralNetwork::evaluate(Mat &image, int classification) {
    reset();

    //initialize the input layer
    Vec3b pixel;
    for (int i = 0; i < image_size; i += 3) {
        pixel = image.at<Vec3b>(i % image.rows, i / image.rows);
        nodes[0][i] = (float)pixel[0] / 255.0;
        nodes[0][i + 1] = (float)pixel[1] / 255.0;
        nodes[0][i + 2] = (float)pixel[2] / 255.0;
    }

    //run the input through the neural newtork
    //edges are sorted by destination layer
    for (int i = 0; i < edges.size(); i++) {
        Edge &e = edges.at(i);
        nodes[e.dst_layer][e.dst_node] += e.weight * nodes[e.src_layer][e.src_node];

        //TODO: implement different node types, ReLU, max pooling, etc.
    }

    double output = nodes[n_layers - 1][0];
    output = fmin( 1.0, output);
    output = fmax(-1.0, output);

    return -(classification - output);
}

double VisionNeuralNetwork::evaluate() {
    double result = 0.0;

    for (int i = 0; i < images.size(); i++) {
        result += evaluate(images[i], classifications[i]);
    }

    return result;
}

double VisionNeuralNetwork::objective_function() {
    return evaluate();
}

double VisionNeuralNetwork::objective_function(const std::vector<double> &parameters) {
    if (edges.size() != parameters.size()) {
        cerr << "Error [" << __FILE__ << ":" << __LINE__ << "]: edges.size (" << edges.size() << ") != parameters.size (" << parameters.size() << ")" << endl; 
        exit(1);
    }

    for (int i = 0; i < edges.size(); i++) {
        edges[i].weight = parameters[i];
    }

    return evaluate();
}

int VisionNeuralNetwork::get_n_edges() {
    return edges.size();
}

void VisionNeuralNetwork::set_edges(const std::vector<Edge> &e) {
    edges.clear();
    edges = e;
}
