#include "edge.hxx"
#include "vision_neural_network.hxx"

#include <cmath>

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include <vector>
using std::vector;


VisionNeuralNetwork::VisionNeuralNetwork(int _image_size, const vector<char*> &_images, const vector<int> &_classifications, int _n_hidden_layers, int _nodes_per_layer) : image_size(_image_size), images(_images), classifications(_classifications) {

    nodes = NULL;
    initialize_nodes(_n_hidden_layers, _nodes_per_layer);
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

    cout << "initializing nodes with n_hidden_layers: " << _n_hidden_layers << " and nodes_per_layer: " << _nodes_per_layer << endl;

    n_hidden_layers = _n_hidden_layers;
    nodes_per_layer = _nodes_per_layer + 1; //add an additional bias node
    n_layers = n_hidden_layers + 2;

    cout << "initializing nodes with n_layer: " << n_layers << " and image_size: " << image_size << endl;
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

double VisionNeuralNetwork::evaluate(const char *image, int classification) {
    reset();

    //initialize the input layer
    for (int i = 0; i < image_size; i++) {
        nodes[0][i] = (float)image[i] / 255.0;
    }

    //run the input through the neural newtork
    //edges are sorted by destination layer
    for (int i = 0; i < edges.size(); i++) {
        Edge &e = edges.at(i);
        nodes[e.dst_layer][e.dst_node] += e.weight * nodes[e.src_layer][e.src_node];

        //TODO: implement bounding on node values.
        //TODO: implement different node types, ReLU, max pooling, etc.
    }

    return fmin(1.0, nodes[n_layers - 1][0] * classification);
}

double VisionNeuralNetwork::evaluate() {
    double result = 0.0;

    for (int i = 0; i < images.size(); i++) {
        result += evaluate(images[i], classifications[i]);
    }

//    cout << "result: " << result << ", images.size(): " << images.size() << endl;

    return result;
}

double VisionNeuralNetwork::objective_function() {
    return evaluate();
}

double VisionNeuralNetwork::objective_function(const vector<double> &parameters) {
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

void VisionNeuralNetwork::set_edges(const vector<Edge> &e) {
    edges.clear();
    edges = e;
}
