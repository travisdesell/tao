#include "convolutional_neural_network.hxx"

#include <algorithm>
using std::random_shuffle;

#include <cmath>
#include <cstdlib>
#include <limits>

#include <iomanip>
using std::setw;

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include <utility>
using std::pair;

#include <vector>
using std::vector;

#ifdef __OPENCL__

#include <cstdio>
#include <cstring>
#include <time.h>

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include "opencl_utils.hxx"

#endif



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

    nodes.push_back( vector< vector<float> >(image_x, vector<float>(image_y)) );
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
        nodes.push_back( vector< vector<float> >(prev_x, vector<float>(prev_y)) );
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

        nodes.push_back( vector< vector<float> >(prev_x, vector<float>(prev_y)) );
        if (!quiet) cout << "created max pooling layer " << (nodes.size() - 1) << " - " << nodes[nodes.size()-1].size() << "x" << nodes[nodes.size()-1][0].size() << ", total_weights: " << total_weights<< endl;
    }

    nodes.push_back( vector <vector<float> >(1, vector<float>(fc_size)) );        //fully connected layer
    total_weights += (prev_x * prev_y) * fc_size;
    total_weights += fc_size; //bias weights
    if (!quiet) cout << "created fully connected layer " << (nodes.size() - 1) << " - " << nodes[nodes.size()-1].size() << "x" << nodes[nodes.size()-1][0].size() << ", total_weights: " << total_weights << endl;

    nodes.push_back( vector <vector<float> >(1, vector<float>(n_classes)) );  //output layer
    total_weights += fc_size * n_classes;
    total_weights += n_classes; //bias weights
    //nodes.push_back( vector <vector<float> >(1, vector<float>(1)) );  //output layer
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

float activation_function(float value) {
//    return fmax(0, fmin(4, value));

     return 1.0f / (1.0f + exp(-value));
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
                float r = image[current] / 256.0f;
                float g = image[current + 1] / 256.0f;
                float b = image[current + 2] / 256.0f;

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
                color_hidden_layer[0] = 1.0 /(1.0 + exp(-color_hidden_layer[0]));
                color_hidden_layer[1] = 1.0 /(1.0 + exp(-color_hidden_layer[1]));
                color_hidden_layer[2] = 1.0 /(1.0 + exp(-color_hidden_layer[2]));
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


                nodes[0][i][j] = activation_function(nodes[0][i][j]);
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
//            cout << "set nodes[0][" << i << "][" << j << "]: " << nodes[0][i][j] << ", rgb: " << rgb << endl;
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
                nodes[out_layer][j][k] = activation_function(nodes[out_layer][j][k]);
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

            if (out_layer == nodes.size() - 1) {
                //softmax layer uses exp instead of sigmoid function
                nodes[out_layer][0][k] = exp(nodes[out_layer][0][k]);
            } else {
                //apply sigmoid function
                nodes[out_layer][0][k] = activation_function(nodes[out_layer][0][k]);
            }
            //printf("fc nodes[%d]: %f\n", k, nodes[out_layer][0][k]);
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
//        printf("output nodes[%d]: %f\n", i, nodes[out_layer][0][i]);
    }
    //cout << endl;

//    cout << "result: " << log(nodes[out_layer][0][classification]) << endl;
    return log(nodes[out_layer][0][classification]);

    //return (nodes[out_layer][0][0] * classification) > 0;
}

double ConvolutionalNeuralNetwork::evaluate() {
    double result = 0.0;

    int total = 0;
    for (int i = 0; i < images.size(); i++) {
        for (int j = 0; j < images[i].size(); j++) {
            double current = evaluate(images[i][j], i) / images[i].size();

            int out_layer = nodes.size() - 1;
            double max_prob = 0.0;
            int max_class;
            for (int k = 0; k < nodes[out_layer][0].size(); k++) {
                if (max_prob < nodes[out_layer][0][k]) {
                    max_prob = nodes[out_layer][0][k];
                    max_class = k;
                }
            }

//            if (max_class == i) current += 0.25;

            result += current / images[i].size();

            total++;
//            cout << "CPU class[" << setw(5) << i << "], image[" << setw(5) << j << "] prob: " << setw(20) << current << endl;
            //if (i == 0) result += evaluate(images[i][j], 1) / images[i].size();
            //else result += evaluate(images[i][j], -1) / images[i].size();
        }
    }

//    cout << "result: " << result << ", images.size(): " << images.size() << endl;

    return result;
//    return result / total;
//    return result / images.size();
}


double ConvolutionalNeuralNetwork::evaluate_stochastic(uint32_t n_samples) {
    double result = 0.0;

    int total = 0;
    for (int i = 0; i < images.size(); i++) {

        if (n_samples > images[i].size()) {
            cerr << "ERROR: on '" << __FILE__ << ":" << __LINE__ << "', n_samples (" << n_samples << ") specified for stochastic evaluation > number of images in class[" << i << "]: " << images[i].size() << endl;
            exit(1);
        }

        //random_shuffle(images[i].begin(), images[i].end());

        for (int j = 0; j < n_samples; j++) {
            uint32_t random_image = drand48() * images[i].size();

            double current = evaluate(images[i][random_image], i) / n_samples;

            int out_layer = nodes.size() - 1;
            double max_prob = 0.0;
            int max_class;
            for (int k = 0; k < nodes[out_layer][0].size(); k++) {
                if (max_prob < nodes[out_layer][0][k]) {
                    max_prob = nodes[out_layer][0][k];
                    max_class = k;
                }
            }

//            if (max_class == i) current += 0.25;

            result += current;

            total++;
//            cout << "CPU class[" << setw(5) << i << "], image[" << setw(5) << j << "] prob: " << setw(20) << current << endl;
            //if (i == 0) result += evaluate(images[i][j], 1) / images[i].size();
            //else result += evaluate(images[i][j], -1) / images[i].size();
        }
    }
    result /= images.size();

//    cout << "result: " << result << ", images.size(): " << images.size() << endl;

    return result;
//    return result / total;
//    return result / images.size();
}


void ConvolutionalNeuralNetwork::print_statistics(const vector<double> &parameters) {
    weights.resize(parameters.size());
    for (int i = 0; i < parameters.size(); i++) weights[i] = parameters[i];


    vector<int> hit_counts(images.size());
    vector< vector<int> > misses(images.size());
    vector< vector<double> > miss_probs(images.size());

    double error = 0.0;
    for (int i = 0; i < images.size(); i++) {
        for (int j = 0; j < images[i].size(); j++) {
            error += evaluate(images[i][j], i) / images[i].size();

            int out_layer = nodes.size() - 1;
            double max_prob = 0.0;
            int max_class;
            for (int k = 0; k < nodes[out_layer][0].size(); k++) {
                if (max_prob < nodes[out_layer][0][k]) {
                    max_prob = nodes[out_layer][0][k];
                    max_class = k;
                }
            }

            //i is the class, if the max prob is the one from that class it's a positive match
            if (max_class == i) {
                hit_counts[i]++;
            } else {
//                misses[i].push_back(j);
//                miss_probs[i].push_back(nodes[out_layer][0][i]);
            }
        }
    }

    error /= images.size();

    cout << "overall error: " << error << endl;
    for (int i = 0; i < images.size(); i++) {
        cout << "class " << i << " hits: " << hit_counts[i] << "/" << images[i].size() << endl;
    }

    /*
    for (int i = 0; i < images.size(); i++) {
        cout << "class " << i << " misses: ";
        for (int j = 0; j < misses[i].size(); j++) {
            cout << " " << misses[i][j] << "(" << miss_probs[i][j] << ")";
        }
        cout << endl;
    }
    cout << endl;
    */
}



double ConvolutionalNeuralNetwork::objective_function() {
    return evaluate();
}

double ConvolutionalNeuralNetwork::objective_function(const vector<double> &parameters) {
    weights.resize(parameters.size());
    for (int i = 0; i < parameters.size(); i++) weights[i] = parameters[i];

    return evaluate();
}

double ConvolutionalNeuralNetwork::objective_function_stochastic(uint32_t n_samples, const vector<double> &parameters) {
    weights.resize(parameters.size());
    for (int i = 0; i < parameters.size(); i++) weights[i] = parameters[i];

    return evaluate_stochastic(n_samples);
}


int ConvolutionalNeuralNetwork::get_n_edges() {
    return total_weights;
}

double ConvolutionalNeuralNetwork::get_output_class(int output_class) {
    return nodes[nodes.size() - 1][0][output_class];
}

void ConvolutionalNeuralNetwork::set_weights(const vector<double> &_weights) {
    weights.resize(_weights.size());
    for (int i = 0; i < _weights.size(); i++) weights[i] = _weights[i];
}



#ifdef __OPENCL__

#define APPLY_KERNEL_FILE "../../tao/neural_networks/convolutional_neural_network_apply_kernel.cl"
#define EVALUATE_KERNEL_FILE "../../tao/neural_networks/convolutional_neural_network_evaluate_kernel.cl"
#define KERNEL_FUNC "add_numbers"

void ConvolutionalNeuralNetwork::initialize_opencl() {
    /* OpenCL structures */
    cl_int  err;

    /* Create device and context */
    device = create_device();
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    check_error(err, "couldn't create a context");

    /* Build program */
    apply_kernel = build_program(context, device, APPLY_KERNEL_FILE);
    evaluate_kernel = build_program(context, device, EVALUATE_KERNEL_FILE);

    vector<char> _images;
    total_images = 0;
    for (int i = 0; i < images.size(); i++) {
        for (int j = 0; j < images[i].size(); j++) {
            for (int k = 0; k < images[i][j].size(); k++) {
                _images.push_back(images[i][j][k]);
            }
            total_images++;
        }
    }

    cout << "total images: " << total_images << ", input size: " << _images.size() << ", values per image: " << (_images.size() / total_images) << endl;

    cout << "loading images" << endl;

    //load the image into constant memory
    image_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, _images.size() * sizeof(char), &(_images[0]), &err);
    check_error(err, "could not load input image into buffer: %d", err);

    cout << "output size == total images: " << total_images << endl;

    output_classifications = new float[total_images];
    for (int i = 0; i < total_images; i++) output_classifications[i] = 0;

    //allocate memory for the output nodes into global memory, should be one per class
    output_classifications_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, total_images * sizeof(float), output_classifications, &err);
    check_error(err, "couldn't create output nodes buffer");


}

double ConvolutionalNeuralNetwork::objective_function_opencl(const vector<double> &parameters) {
    weights.resize(parameters.size());
    for (int i = 0; i < parameters.size(); i++) weights[i] = parameters[i];

    return evaluate_opencl();
}

double ConvolutionalNeuralNetwork::evaluate_opencl() {
    reset();

    cl_int err;

    //load nn parameters into constant memory
    int nn_params_size = 6 + (layers.size() * 2) + n_classes;
    int nn_params[nn_params_size];
    nn_params[0] = image_x;
    nn_params[1] = image_y;
    nn_params[2] = layers.size();
    nn_params[3] = fc_size;
    nn_params[4] = n_classes;

    for (int i = 0; i < layers.size(); i++) {
        nn_params[5 + (i * 2)    ] = layers[i].first;   // conv size
        nn_params[5 + (i * 2) + 1] = layers[i].second;  // max pool size
    }

    int sum = 0;
    for (int i = 0; i < images.size(); i++) {
        sum += images[i].size();
        nn_params[5 + (layers.size() * 2) + i] = sum;   //where the classes start
    }

    cl_mem nn_params_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, nn_params_size * sizeof(int), &(nn_params[0]), &err);
    check_error(err, "could not load nn_parameters into buffer: %d", err);

    float _weights[weights.size()];
    for (int i = 0; i < weights.size(); i++) _weights[i] = weights[i];

    //load weights into constant memory
    cl_mem weights_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, weights.size() * sizeof(float), &(_weights[0]), &err);
    check_error(err, "could not load conv_net weights into buffer: %d", err);

    /* Create a command queue */
    queue = clCreateCommandQueue(context, device, 0, &err);
    check_error(err, "couldn't create a command queue: %d", err);

    /* Create a kernel */
    kernel = clCreateKernel(evaluate_kernel, KERNEL_FUNC, &err);
    check_error(err, "couldn't create a kernel");

    /* Create kernel arguments */
    err =  clSetKernelArg(kernel, 0, sizeof(cl_mem), &image_buffer);
    check_error(err, "couldn't create input image argument: %d", err);

    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &nn_params_buffer);
    check_error(err, "couldn't create nn_params argument: %d", err);

    err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &weights_buffer);
    check_error(err, "couldn't create input weights argument: %d", err);

    err = clSetKernelArg(kernel, 3, image_x * image_y * sizeof(float), NULL);
    check_error(err, "couldn't create local nodes argument: %d", err);

    err = clSetKernelArg(kernel, 4, fc_size * sizeof(float), NULL);
    check_error(err, "couldn't create fully connected nodes argument: %d", err);

    err = clSetKernelArg(kernel, 5, sizeof(cl_mem), &output_classifications_buffer);
    check_error(err, "couldn't create group result kernel argument: %d", err);

    //size_t local_size = 64;
    //size_t global_size = local_size * ceil((float)total_images / local_size);

    size_t global_size = total_images;
    size_t local_size = 1;

    /* Enqueue kernel */
    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL); 
    check_error(err, "couldn't enqueue the kernel");

    /* Read the kernel's output */
    err = clEnqueueReadBuffer(queue, output_classifications_buffer, CL_TRUE, 0, total_images * sizeof(float), &(output_classifications[0]), 0, NULL, NULL);
    check_error(err, "couldn't read the output nodes buffer: %d", err);

    clReleaseMemObject(weights_buffer);

    double result = 0.0;
    int pos = 0; 
    for (int i = 0; i < images.size(); i++) {
        for (int j = 0; j < images[i].size(); j++) {
            double current = output_classifications[pos] / images[i].size();
//            cout << "GPU class[" << setw(5) << i << "], image[" << setw(5) << j << "] prob: " << setw(20) << current << ", pos: " << pos << ", " << total_images << endl;
            result += current;
            pos++;
        }
    }

    return result / images.size();
}


vector<float> ConvolutionalNeuralNetwork::apply_to_image_opencl(const vector<char> &image, int rows, int cols, int classification) {
    reset();

    cl_int err;
    vector<char> _image(image);

    cout << "input size: " << rows << "x" << cols << "x" << 3 << " = " << _image.size() << endl;

    float _weights[weights.size()];
    for (int i = 0; i < weights.size(); i++) _weights[i] = weights[i];

    //load the image into constant memory
    cl_mem image_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, _image.size() * sizeof(char), &(_image[0]), &err);
    check_error(err, "could not load input image into buffer: %d", err);

    //load weights into constant memory
    cl_mem weights_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, weights.size() * sizeof(float), &(_weights[0]), &err);
    check_error(err, "could not load conv_net weights into buffer: %d", err);

    int output_size = (rows - image_y) * (cols - image_x);
    cout << "output size: " << (rows - image_y) << "x" << (cols - image_x) << " = " << output_size << endl;
    float output_image[output_size];
    for (int i = 0; i < output_size; i++) output_image[i] = 0;
    //allocate memory for the output nodes into global memory, should be one per class
    cl_mem output_image_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, output_size * sizeof(float), output_image, &err);
    check_error(err, "couldn't create output nodes buffer");

    /* Create a command queue */
    queue = clCreateCommandQueue(context, device, 0, &err);
    check_error(err, "couldn't create a command queue: %d", err);

    /* Create a kernel */
    kernel = clCreateKernel(apply_kernel, KERNEL_FUNC, &err);
    check_error(err, "couldn't create a kernel");

    /* Create kernel arguments */
    err =  clSetKernelArg(kernel, 0, sizeof(cl_mem), &image_buffer);
    check_error(err, "couldn't create input image argument: %d", err);

    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &weights_buffer);
    check_error(err, "couldn't create input weights argument: %d", err);

    err = clSetKernelArg(kernel, 2, image_x * image_y * sizeof(float), NULL);
    check_error(err, "couldn't create local nodes argument: %d", err);

    err = clSetKernelArg(kernel, 3, fc_size * sizeof(float), NULL);
    check_error(err, "couldn't create fully connected nodes argument: %d", err);

    err = clSetKernelArg(kernel, 4, sizeof(cl_mem), &output_image_buffer);
    check_error(err, "couldn't create group result kernel argument: %d", err);

    size_t *global_size = (size_t*)malloc(sizeof(size_t) * 2);
    size_t *local_size = (size_t*)malloc(sizeof(size_t) * 2);

    global_size[0] = cols - image_x, global_size[1] = rows - image_y;
    local_size[0] = 1, local_size[1] = 1;

//    size_t local_size = (rows - image_y);
    /* Enqueue kernel */
    err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_size, local_size, 0, NULL, NULL); 
    check_error(err, "couldn't enqueue the kernel");

    /* Read the kernel's output */
    err = clEnqueueReadBuffer(queue, output_image_buffer, CL_TRUE, 0, output_size * sizeof(float), &output_image, 0, NULL, NULL);
    check_error(err, "couldn't read the output nodes buffer: %d", err);

    clReleaseMemObject(image_buffer);
    clReleaseMemObject(weights_buffer);
    clReleaseMemObject(output_image_buffer);

    for (int i = 0; i < output_size; i++) {
        cout << output_image[i] << endl;
    }
    vector<float> result_image(output_image, output_image + output_size);

    return result_image;
}

void ConvolutionalNeuralNetwork::deinitialize_opencl() {
    /* Deallocate resources */
    clReleaseMemObject(image_buffer);
    clReleaseMemObject(output_classifications_buffer);
    delete [] output_classifications;

    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseProgram(apply_kernel);
    clReleaseProgram(evaluate_kernel);
    clReleaseContext(context);
}
#endif


