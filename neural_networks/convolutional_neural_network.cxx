#include "convolutional_neural_network.hxx"

#include <cmath>
#include <cstdlib>

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include <utility>
using std::pair;

#include <vector>
using std::vector;


ConvolutionalNeuralNetwork::ConvolutionalNeuralNetwork(int _image_x, int _image_y, bool _rgb, const vector< vector< vector<char> > > &_images, const vector< pair<int, int> > &_layers, int _fc_size) : image_x(_image_x), image_y(_image_y), rgb(_rgb), images(_images) {

    n_classes = _images.size();

    initialize_nodes(_layers, _fc_size);
}

void ConvolutionalNeuralNetwork::initialize_nodes(const vector< pair<int, int> > &_layers, int _fc_size) {
    nodes.clear();

    layers = vector< pair< int, int> >(_layers);
    fc_size = _fc_size;
    n_classes = images.size();

    cout << "initializing nodes with: " << endl;
    for (int i = 0; i < layers.size(); i++) {
        cout << "  layer " << i << " -- convolutional: " << layers[i].first << "x" << layers[i].first << ", max pooling: " << layers[i].second << "x" << layers[i].second << endl;
    }
    cout << "  layer " << layers.size() << " -- fully connected: " << fc_size << endl;
    cout << "  output layer -- classes: " << n_classes << endl;

    total_weights = 0;
    //if (rgb) total_weights = 3;
    if (rgb) total_weights = 16;

    nodes.push_back( vector< vector<double> >(image_x, vector<double>(image_y)) );
    cout << "created input layer " << (nodes.size() - 1) << " - " << nodes[nodes.size()-1].size() << "x" << nodes[nodes.size()-1][0].size() << endl;

    int prev_x = image_x, prev_y = image_y;
    for (int i = 0; i < layers.size(); i++) {
        cout << "creating convolutional layer " << nodes.size() << ", total_weights: " << total_weights << endl;

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

        cout << "created convolutional layer " << (nodes.size() - 1) << " - " << nodes[nodes.size()-1].size() << "x" << nodes[nodes.size()-1][0].size() << endl;

        cout << "creating max pooling layer " << nodes.size() << ", total_weights: " << total_weights << endl;
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
        total_weights += (prev_x * prev_y);

        nodes.push_back( vector< vector<double> >(prev_x, vector<double>(prev_y)) );
        cout << "created max pooling layer " << (nodes.size() - 1) << " - " << nodes[nodes.size()-1].size() << "x" << nodes[nodes.size()-1][0].size() << endl;
    }

    nodes.push_back( vector <vector<double> >(1, vector<double>(fc_size)) );        //fully connected layer
    total_weights += (prev_x * prev_y) * fc_size;
    total_weights += fc_size; //bias weights
    cout << "created fully connected layer " << (nodes.size() - 1) << " - " << nodes[nodes.size()-1].size() << "x" << nodes[nodes.size()-1][0].size() << ", total_weights: " << total_weights << endl;

//    nodes.push_back( vector <vector<double> >(1, vector<double>(n_classes)) );  //output layer
//    total_weights += fc_size * n_classes;
    nodes.push_back( vector <vector<double> >(1, vector<double>(1)) );  //output layer
    total_weights += fc_size;
    total_weights += 1; //bias weight;

    cout << "created output layer " << (nodes.size() - 1) << " - " << nodes[nodes.size()-1].size() << "x" << nodes[nodes.size()-1][0].size() << ", total_weights: " << total_weights << endl;

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

double convert(char val) {
    return (double)((short)val + 128.0) / 255.0;
}

double ConvolutionalNeuralNetwork::evaluate(const vector<char> &image, int classification) {
    reset();

    double color_hidden_layer[3];

    //cout << "initializing input layer" << endl;
    //set input layer
    int current = 0;
    for (int i = 0; i < image_x; i++) {
        for (int j = 0; j < image_y; j++) {
            if (rgb) {
                color_hidden_layer[0]  = weights[0] * convert(image[current]);
                color_hidden_layer[0] += weights[1] * convert(image[current + 1]);
                color_hidden_layer[0] += weights[2] * convert(image[current + 2]);

                color_hidden_layer[1]  = weights[3] * convert(image[current]);
                color_hidden_layer[1] += weights[4] * convert(image[current + 1]);
                color_hidden_layer[1] += weights[5] * convert(image[current + 2]);

                color_hidden_layer[2]  = weights[6] * convert(image[current]);
                color_hidden_layer[2] += weights[7] * convert(image[current + 1]);
                color_hidden_layer[2] += weights[8] * convert(image[current + 2]);

                //bias
                color_hidden_layer[0] += weights[9];
                color_hidden_layer[0] += weights[10];
                color_hidden_layer[0] += weights[11];

                nodes[0][i][j]  = weights[12] * convert(color_hidden_layer[0]);
                nodes[0][i][j] += weights[13] * convert(color_hidden_layer[1]);
                nodes[0][i][j] += weights[14] * convert(color_hidden_layer[2]);

                //bias
                nodes[0][i][j] += weights[15];
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
//            cout << "set nodes[0][" << i << "][" << j << "]: " << nodes[0][i][j] << ", rgb: " << rgb << endl;
        }
    }

    int current_weight = 0;
//    if (rgb) current_weight = 3;
    if (rgb) current_weight = 16;

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

                if (nodes[out_layer][j][k] > 10.0) nodes[out_layer][j][k] = 10.0;
                //cout << "calculated nodes[" << out_layer << "][" << j << "][" << k << "]: " << nodes[out_layer][j][k] << endl;
            }
        }
        //current_weight += (conv_size * conv_size);
        current_weight = bias_weight;

        int max_pool_size = layers[i].second;
        in_layer++;
        out_layer++;

        bias_weight = current_weight;
        //cout << "calculating input layer " << in_layer << "(" << nodes[in_layer].size() << "x" << nodes[in_layer][0].size() << ") to output layer " << out_layer << " (" << nodes[out_layer].size() << "x" << nodes[out_layer][0].size() << ")" << endl;
        for (int j = 0; j < nodes[in_layer].size() / max_pool_size; j++) {
            for (int k = 0; k < nodes[in_layer][j].size() / max_pool_size; k++) {
                
                //cout << "calculating nodes[" << out_layer << "][" << j << "][" << k << "]: " << endl;
                for (int l = 0; l < max_pool_size; l++) {
                    for (int m = 0; m < max_pool_size; m++) {
                        double val = nodes[in_layer][(j * max_pool_size) + l][(k * max_pool_size) + m];
                        if (nodes[out_layer][j][k] < val) nodes[out_layer][j][k] = val;
                    }
                }

                nodes[out_layer][j][k] += weights[bias_weight];
                bias_weight++;

                if (nodes[out_layer][j][k] > 10.0) nodes[out_layer][j][k] = 10.0;
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
        }
    }
    
    if (current_weight != total_weights) {
        cout << "ERROR: current weight " << current_weight << " != total_weights " << total_weights << endl;
        exit(1);
    }

    out_layer = nodes.size() - 1;
    int max_output = 0;
    double max_output_val = nodes[out_layer][0][0];
    for (int i = 1; i < nodes[out_layer].size(); i++) {
        double val = nodes[out_layer][0][i];
        if (val > max_output_val) {
            max_output_val = val;
            max_output = i;
        }
    }

    /*
    //return 0 if the classification was right, otherwise return
    //the distance between the max output_val.
    if (max_output == classification) {
//        cout << "output_nodes: " << nodes[out_layer][0][0] << ", " << nodes[out_layer][0][1] << endl;
        return max_output - 1.0;
    } else {
        //max_outut_val > the classification output, so this will be
        //negative (which is what we want for maximization)
        //cout << "classification: " << classification << ", max_output_node: " << max_output << ", max_output_val: " << max_output_val << ", output_val: " << nodes[out_layer][0][classification] << endl;
        return  -fabs( nodes[out_layer][0][classification] - max_output_val );
    }
    */

    return (nodes[out_layer][0][0] * classification) > 0;
//    return fmin(1.0, nodes[out_layer][0][0] * classification);

}

double ConvolutionalNeuralNetwork::evaluate() {
    double result = 0.0;

    for (int i = 0; i < images.size(); i++) {
        for (int j = 0; j < images[i].size(); j++) {
            /*
            double val;
            if (i == 0) val = evaluate(images[i][j], 1);
            else val = evaluate(images[i][j], -1);
            result += (val / images[i].size());
            */
            if (i == 0) result += evaluate(images[i][j], 1) / images[i].size();
            else result += evaluate(images[i][j], -1) / images[i].size();
        }
    }

//    cout << "result: " << result << ", images.size(): " << images.size() << endl;

    return result / images.size();
}

void ConvolutionalNeuralNetwork::print_statistics(const vector<double> &parameters) {
    weights = vector<double>(parameters);

    int true_positives = 0, true_negatives = 0;
    vector<int> positive_misses;
    vector<int> negative_misses;

    for (int i = 0; i < images.size(); i++) {
        for (int j = 0; j < images[i].size(); j++) {
            if (i == 0) {
                if (evaluate(images[i][j], 1) > 0) {
                    true_positives++;
                } else {
                    positive_misses.push_back(j);
                }

            } else {
                if (evaluate(images[i][j], -1) > 0) {
                    true_negatives++;
                } else {
                    negative_misses.push_back(j);
                }
            }
        }
    }

    cout << "true positives: " << true_positives << "/" << images[0].size() << ", true negatives: " << true_negatives << "/" << images[1].size() << endl;

    cout << "    positive misses:";
    for (int i = 0; i < positive_misses.size(); i++) {
        cout << " " << positive_misses[i];
    }
    cout << endl;

    cout << "    negative misses:";
    for (int i = 0; i < negative_misses.size(); i++) {
        cout << " " << negative_misses[i];
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
