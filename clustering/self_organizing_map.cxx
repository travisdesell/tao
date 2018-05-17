


typedef double (*distance_function_1d)(const vector<double> &sample1, const vector<double> &sample2);
typedef double (*distance_function_2d)(const vector< vector<double> > &sample1, const vector< vector<double> > &sample2);


void self_organizing_map(uint32_t number_iterations, const vector< vector<double> > &samples, distance_function_1d distance) {
    /**
     *  Each sample should have the same number of time series
     */
    for (uint32_t i = 0; i < samples.size(); i++) {

    }

    /**
     *  Number of elements in neurons should be the average number of neurons in the samples.
     *  Values should be assigned randomly between min and max values of each time series.
     */
    vector< vector<double> > neurons;

    for (uint32_t i = 0; i < number_iterations; i++) {

    }

}
