#include "convolutional_neural_network.hxx"

#include <cmath>
#include <cstdlib>
#include <limits>

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include <utility>
using std::pair;

#include <vector>
using std::vector;


ConvolutionalNeuralNetwork::ConvolutionalNeuralNetwork(int _image_x, int _image_y, bool _rgb, bool _quiet, const vector< vector< vector<char> > > &_images, const vector< pair<int, int> > &_layers, int _fc_size) : image_x(_image_x), image_y(_image_y), rgb(_rgb), images(_images) {

    quiet = _quiet;
    n_classes = _images.size();

    initialize_nodes(_layers, _fc_size);
}

void ConvolutionalNeuralNetwork::initialize_nodes(const vector< pair<int, int> > &_layers, int _fc_size) {
    nodes.clear();

    layers = vector< pair< int, int> >(_layers);
    fc_size = _fc_size;
    n_classes = images.size();

    if (!quiet) {
        cout << "initializing nodes with: " << endl;
        for (int i = 0; i < layers.size(); i++) {
            cout << "  layer " << i << " -- convolutional: " << layers[i].first << "x" << layers[i].first << ", max pooling: " << layers[i].second << "x" << layers[i].second << endl;
        }
        cout << "  layer " << layers.size() << " -- fully connected: " << fc_size << endl;
        cout << "  output layer -- classes: " << n_classes << endl;
    }

    total_weights = 0;
    if (rgb) total_weights = 4;
    //if (rgb) total_weights = 15;

    nodes.push_back( vector< vector<double> >(image_x, vector<double>(image_y)) );
    if (!quiet) cout << "created input layer " << (nodes.size() - 1) << " - " << nodes[nodes.size()-1].size() << "x" << nodes[nodes.size()-1][0].size() << endl;

    int prev_x = image_x, prev_y = image_y;
    for (int i = 0; i < layers.size(); i++) {
        if (prev_x - layers[i].second <= 0) {
            cout << "ERROR creating convolutional layer[" << i << "], input_x: " << prev_x << " % " << layers[i].second << " <= 0" << endl;
            exit(1);
        }

        if (prev_y - layers[i].second <= 0) {
            cout << "ERROR creating convolutional layer[" << i << "], input_y: " << prev_y << " % " << layers[i].second << " <= 0" << endl;
            exit(1);
        }

        prev_x -= layers[i].first;
        prev_y -= layers[i].first;
        nodes.push_back( vector< vector<double> >(prev_x, vector<double>(prev_y)) );
        total_weights += (layers[i].first * layers[i].first); //convolutional layer weights
        total_weights += (prev_x * prev_y); //bias weights

        if (!quiet) cout << "created convolutional layer " << (nodes.size() - 1) << " - " << nodes[nodes.size()-1].size() << "x" << nodes[nodes.size()-1][0].size() << ", total_weights: " << total_weights << endl;
        if (prev_x % layers[i].second > 0) {
            cout << "ERROR creating max pooling layer[" << i << "], input_x: " << prev_x << " % " << layers[i].second << " > 0" << endl;
            exit(1);
        }

        if (prev_y % layers[i].second > 0) {
            cout << "ERROR creating max pooling layer[" << i << "], input_y: " << prev_y << " % " << layers[i].second << " > 0" << endl;
            exit(1);
        }

        prev_x /= layers[i].second;
        prev_y /= layers[i].second;
        //no weights on max pooling 
        //total_weights += (layers[i].second * layers[i].second);
        //total_weights += (prev_x * prev_y);

        nodes.push_back( vector< vector<double> >(prev_x, vector<double>(prev_y)) );
        if (!quiet) cout << "created max pooling layer " << (nodes.size() - 1) << " - " << nodes[nodes.size()-1].size() << "x" << nodes[nodes.size()-1][0].size() << ", total_weights: " << total_weights<< endl;
    }

    nodes.push_back( vector <vector<double> >(1, vector<double>(fc_size)) );        //fully connected layer
    total_weights += (prev_x * prev_y) * fc_size;
    total_weights += fc_size; //bias weights
    if (!quiet) cout << "created fully connected layer " << (nodes.size() - 1) << " - " << nodes[nodes.size()-1].size() << "x" << nodes[nodes.size()-1][0].size() << ", total_weights: " << total_weights << endl;

    nodes.push_back( vector <vector<double> >(1, vector<double>(n_classes)) );  //output layer
    total_weights += fc_size * n_classes;
    total_weights += n_classes; //bias weights
    //nodes.push_back( vector <vector<double> >(1, vector<double>(1)) );  //output layer
    //total_weights += fc_size;
    //total_weights += 1; //bias weight;

    if (!quiet) cout << "created output layer " << (nodes.size() - 1) << " - " << nodes[nodes.size()-1].size() << "x" << nodes[nodes.size()-1][0].size() << ", total_weights: " << total_weights << endl;

    reset();
}

void ConvolutionalNeuralNetwork::reset() {
    for (int i = 0; i < nodes.size(); i++) {
        for (int j = 0; j < nodes[i].size(); j++) {
            for (int k = 0; k < nodes[i][j].size(); k++) {
                nodes[i][j][k] = 0.0;
            }
        }
    }
}

double ConvolutionalNeuralNetwork::evaluate(const vector<char> &image, int classification) {
    reset();

    //double color_hidden_layer[3];

    //cout << "initializing input layer" << endl;
    //set input layer
    int current = 0;
    for (int i = 0; i < image_x; i++) {
        for (int j = 0; j < image_y; j++) {
            if (rgb) {
                double r = image[current] / 256.0;
                double g = image[current + 1] / 256.0;
                double b = image[current + 2] / 256.0;

                /*
                color_hidden_layer[0]  = weights[0] * r;
                color_hidden_layer[0] += weights[1] * g;
                color_hidden_layer[0] += weights[2] * b;
                //cout << "color_hidden_layer[0]: " << color_hidden_layer[0] << endl;
                

                color_hidden_layer[1]  = weights[3] * r;
                color_hidden_layer[1] += weights[4] * g;
                color_hidden_layer[1] += weights[5] * b;
                //cout << "color_hidden_layer[1]: " << color_hidden_layer[1] << endl;

                color_hidden_layer[2]  = weights[6] * r;
                color_hidden_layer[2] += weights[7] * g;
                color_hidden_layer[2] += weights[8] * b;
                //cout << "color_hidden_layer[2]: " << color_hidden_layer[2] << endl;

                //bias
                color_hidden_layer[0] += weights[9];
                color_hidden_layer[1] += weights[10];
                color_hidden_layer[2] += weights[11];

                //apply sigmoid function
                color_hidden_layer[0] = 1.0 /(1.0 + exp(color_hidden_layer[0]));
                color_hidden_layer[1] = 1.0 /(1.0 + exp(color_hidden_layer[1]));
                color_hidden_layer[2] = 1.0 /(1.0 + exp(color_hidden_layer[2]));
                //cout << "color_hidden_layer[0]: " << color_hidden_layer[0] << endl;
                //cout << "color_hidden_layer[1]: " << color_hidden_layer[1] << endl;
                //cout << "color_hidden_layer[2]: " << color_hidden_layer[2] << endl;

                nodes[0][i][j]  = weights[12] * color_hidden_layer[0];
                nodes[0][i][j] += weights[13] * color_hidden_layer[1];
                nodes[0][i][j] += weights[14] * color_hidden_layer[2];
                */

                nodes[0][i][j]  = weights[0] * r;
                nodes[0][i][j] += weights[1] * g;
                nodes[0][i][j] += weights[2] * b;
                nodes[0][i][j] += weights[3];


                nodes[0][i][j] = 1.0 / (1.0 + exp(nodes[0][i][j]));
                //bias
//                nodes[0][i][j] += weights[15];
                current += 3;

                /*
                nodes[0][i][j]  = weights[0] * convert(image[current]);
                nodes[0][i][j] += weights[1] * convert(image[current + 1]);
                nodes[0][i][j] += weights[2] * convert(image[current + 2]);
                current += 3;
                */
            } else {
                nodes[0][i][j] = image[current];
                current++;
            }
            //cout << "set nodes[0][" << i << "][" << j << "]: " << nodes[0][i][j] << ", rgb: " << rgb << endl;
        }
    }

    int current_weight = 0;
    if (rgb) current_weight = 4;
    //if (rgb) current_weight = 15;

    int in_layer = 0, out_layer = 1;

    //cout << "calculating convolutional/max pooling layers" << endl;
    //cout << "layers.size: " << layers.size() << endl;
    //do the convolutional/max pooling layers
    for (int i = 0; i < layers.size(); i++) {
        //cout << "getting conv_size of layers[" << i << "]: " << layers[i].first << endl;
        int conv_size = layers[i].first;

        //cout << "calculating input layer " << in_layer << "(" << nodes[in_layer].size() << "x" << nodes[in_layer][0].size() << ") to output layer " << out_layer << " (" << nodes[out_layer].size() << "x" << nodes[out_layer][0].size() << ")" << endl;

        int bias_weight = current_weight + (conv_size * conv_size);
        for (int j = 0; j < nodes[in_layer].size() - conv_size; j++) {
            for (int k = 0; k < nodes[in_layer][j].size() - conv_size; k++) {

                //cout << "calculating nodes[" << out_layer << "][" << j << "][" << k << "]: " << endl;
                for (int l = 0; l < conv_size; l++) {
                    for (int m = 0; m < conv_size; m++) {
                        nodes[out_layer][j][k] += weights[current_weight + (l * conv_size) + m] * nodes[in_layer][j + l][k + m];
                    }
                }

                nodes[out_layer][j][k] += weights[bias_weight];
                bias_weight++;

                //if (nodes[out_layer][j][k] > 10.0) nodes[out_layer][j][k] = 10.0;
                nodes[out_layer][j][k] = 1.0 / (1.0 + exp(nodes[out_layer][j][k]));
                //nodes[out_layer][j][k] = fmax(0.0, nodes[out_layer][j][k]);
                
                //cout << "calculated nodes[" << out_layer << "][" << j << "][" << k << "]: " << nodes[out_layer][j][k] << endl;
            }
        }
        //current_weight += (conv_size * conv_size);
        current_weight = bias_weight;

        int max_pool_size = layers[i].second;
        in_layer++;
        out_layer++;

        bias_weight = current_weight;
        //bias_weight = current_weight + (max_pool_size * max_pool_size);
        //cout << "calculating input layer " << in_layer << "(" << nodes[in_layer].size() << "x" << nodes[in_layer][0].size() << ") to output layer " << out_layer << " (" << nodes[out_layer].size() << "x" << nodes[out_layer][0].size() << ")" << endl;
        for (int j = 0; j < nodes[in_layer].size() / max_pool_size; j++) {
            for (int k = 0; k < nodes[in_layer][j].size() / max_pool_size; k++) {
                
                //cout << "calculating nodes[" << out_layer << "][" << j << "][" << k << "]: " << endl;
                nodes[out_layer][j][k] = -std::numeric_limits<double>::max();
                for (int l = 0; l < max_pool_size; l++) {
                    for (int m = 0; m < max_pool_size; m++) {
                        double val = nodes[in_layer][(j * max_pool_size) + l][(k * max_pool_size) + m];
                        //double val = weights[current_weight + (l * max_pool_size) + m] * nodes[in_layer][(j * max_pool_size) + l][(k * max_pool_size) + m];
                        if (nodes[out_layer][j][k] < val) nodes[out_layer][j][k] = val;
                    }
                }

                //nodes[out_layer][j][k] += weights[bias_weight];
                //bias_weight++;

                //if (nodes[out_layer][j][k] > 10.0) nodes[out_layer][j][k] = 10.0;
                //cout << "calculated nodes[" << out_layer << "][" << j << "][" << k << "]: " << nodes[out_layer][j][k] << endl;
            }
        }
        //current_weight += (max_pool_size * max_pool_size);
        current_weight = bias_weight;
        in_layer++;
        out_layer++;
    }


    //cout << "calculating the fully connected layer and output layer" << endl;
    //do the fully connected layers and output layer
    for (; in_layer < nodes.size() - 1; in_layer++, out_layer++) {

        for (int k = 0; k < nodes[out_layer][0].size(); k++) {
            for (int i = 0; i < nodes[in_layer].size(); i++) {
                for (int j = 0; j < nodes[in_layer][i].size(); j++) {

                    nodes[out_layer][0][k] += weights[current_weight] * nodes[in_layer][i][j];

                    current_weight++;
                }
            }

            nodes[out_layer][0][k] += weights[current_weight]; //bias
            current_weight++;

            //apply sigmoid function
            nodes[out_layer][0][k] = 1.0 / (1.0 + exp(nodes[out_layer][0][k]));
        }
    }

    if (current_weight != total_weights) {
        cout << "ERROR: current weight " << current_weight << " != total_weights " << total_weights << endl;
        exit(1);
    }

    out_layer = nodes.size() - 1;
    double sum = 0.0;
    for (int i = 0; i < nodes[out_layer][0].size(); i++) {
        sum += nodes[out_layer][0][i];
    }

    //normalize outputs
    //cout << "output: ";
    for (int i = 0; i < nodes[out_layer][0].size(); i++) {
        nodes[out_layer][0][i] /= sum;
        //cout << " " << nodes[out_layer][0][i];
    }
    //cout << endl;

    return log(nodes[out_layer][0][classification]);

    //return (nodes[out_layer][0][0] * classification) > 0;
}

double ConvolutionalNeuralNetwork::evaluate() {
    double result = 0.0;

    for (int i = 0; i < images.size(); i++) {
        for (int j = 0; j < images[i].size(); j++) {
            result += evaluate(images[i][j], i) / images[i].size();

            //if (i == 0) result += evaluate(images[i][j], 1) / images[i].size();
            //else result += evaluate(images[i][j], -1) / images[i].size();
        }
    }

//    cout << "result: " << result << ", images.size(): " << images.size() << endl;

    return result / images.size();
}

void ConvolutionalNeuralNetwork::print_statistics(const vector<double> &parameters) {
    weights = vector<double>(parameters);

    vector<int> hit_counts(images.size());
    vector< vector<int> > misses(images.size());
    vector< vector<double> > miss_probs(images.size());

    for (int i = 0; i < images.size(); i++) {
        for (int j = 0; j < images[i].size(); j++) {
            evaluate(images[i][j], i);

            int out_layer = nodes.size() - 1;
            double max_prob = 0.0;
            for (int k = 0; k < nodes[out_layer][0].size(); k++) {
                if (max_prob < nodes[out_layer][0][k]) max_prob = nodes[out_layer][0][k];
            }

            //i is the class, if the max prob is the one from that class it's a positive match
            if (max_prob == nodes[out_layer][0][i]) {
                hit_counts[i]++;
            } else {
                misses[i].push_back(j);
                miss_probs[i].push_back(nodes[out_layer][0][i]);
            }
        }
    }

    for (int i = 0; i < images.size(); i++) {
        cout << "class " << i << " hits: " << hit_counts[i] << "/" << images[i].size() << endl;
    }

    for (int i = 0; i < images.size(); i++) {
        cout << "class " << i << " misses: ";
        for (int j = 0; j < misses[i].size(); j++) {
            cout << " " << misses[i][j] << "(" << miss_probs[i][j] << ")";
        }
        cout << endl;
    }
    cout << endl;
}



double ConvolutionalNeuralNetwork::objective_function() {
    return evaluate();
}

double ConvolutionalNeuralNetwork::objective_function(const vector<double> &parameters) {
    weights = vector<double>(parameters);
    return evaluate();
}

int ConvolutionalNeuralNetwork::get_n_edges() {
    return total_weights;
}

double ConvolutionalNeuralNetwork::get_output_class(int output_class) {
    return nodes[nodes.size() - 1][0][output_class];
}

void ConvolutionalNeuralNetwork::set_weights(const vector<double> &_weights) {
    weights = vector<double>(_weights);
}
