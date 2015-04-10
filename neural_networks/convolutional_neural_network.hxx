#ifndef TAO_CONVOLUTIONAL_NEURAL_NETWORK_H
#define TAO_CONVOLUTIONAL_NEURAL_NETWORK_H

#include <utility>
using std::pair;

#include <vector>
using std::vector;

#ifdef __OPENCL__

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#endif

class ConvolutionalNeuralNetwork {
    private:
        int image_x, image_y;
        bool rgb;
        bool quiet;

        vector< vector< vector<char> > > images;

        vector< pair<int, int> > layers;

        int fc_size;
        int n_classes;

        int total_weights;
        vector< vector< vector<float> > > nodes;

        vector<float> weights;

#ifdef __OPENCL__
        cl_device_id device;
        cl_context context;
        cl_program apply_kernel;
        cl_program evaluate_kernel;
        cl_kernel kernel;
        cl_command_queue queue;

        cl_mem image_buffer;
        cl_mem output_classifications_buffer;

        int total_images;
        float *output_classifications;
#endif

    public:
        ConvolutionalNeuralNetwork(int _image_x, int _image_y, bool _rgb, bool _quiet, const vector< vector< vector<char> > > &_images, const vector< pair<int, int> > &_layers, int _fc_size);


        void initialize_nodes(const vector< pair<int, int> > &_layers, int _fc_size);
 
        int get_n_edges();
        void set_weights(const vector<double> &_weights);

        void reset();

        double get_output_class(int output_class);

        double objective_function();
        double objective_function(const vector<double> &parameters);
        double objective_function_stochastic(uint32_t n_samples, const vector<double> &parameters);


        double evaluate();
        double evaluate(const vector<char> &image, int classification);
        double evaluate_stochastic(uint32_t n_samples);

        void print_statistics(const vector<double> &parameters);


#ifdef __OPENCL__
        void initialize_opencl();
        void deinitialize_opencl();

        vector<float> apply_to_image_opencl(const vector<char> &image, int rows, int cols, int classification);
        double evaluate_opencl();
        double objective_function_opencl(const vector<double> &parameters);
#endif
};

#endif
