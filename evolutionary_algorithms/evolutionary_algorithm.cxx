#include <iostream>
#include <vector>
#include <string>

#include <stdint.h>

#include "evolutionary_algorithm.hxx"
#include "recombination.hxx"

//from undvc_common
#include "arguments.hxx"

using namespace std;

EvolutionaryAlgorithm::EvolutionaryAlgorithm( const vector<double> &min_bound,      /* min bound is copied into the search */
                                              const vector<double> &max_bound,      /* max bound is copied into the search */
                                              const vector<string> &arguments       /* initialize the DE from command line arguments */
                                            ) throw (string) {
    current_iteration = 0;
    evaluations_done = 0;
    maximum_iterations = 0;

    if (!get_argument(arguments, "--population_size", false, population_size)) {
        cerr << "Argument '--population_size' not specified, using default of 50." << endl;
        population_size = 50;
    }

    if (!get_argument(arguments, "--maximum_iterations", false, maximum_iterations)) {
        cerr << "Argument '--maximum_iterations' not specified, running forever. Hit control-C to quit." << endl;
    }

    Recombination::check_bounds(min_bound, max_bound);

    number_parameters = min_bound.size();
    this->min_bound = vector<double>(min_bound);
    this->max_bound = vector<double>(max_bound);
}

EvolutionaryAlgorithm::EvolutionaryAlgorithm( const vector<double> &min_bound,      /* min bound is copied into the search */
                                              const vector<double> &max_bound,      /* max bound is copied into the search */
                                              const uint32_t population_size,
                                              const uint32_t maximum_iterations     /* default value is 0, which means no termination */
                                            ) throw (string) {
    this->population_size = population_size;

    current_iteration = 0;
    evaluations_done = 0;
    this->maximum_iterations = maximum_iterations;

    Recombination::check_bounds(min_bound, max_bound);

    number_parameters = min_bound.size();
    this->min_bound = vector<double>(min_bound);
    this->max_bound = vector<double>(max_bound);
}

EvolutionaryAlgorithm::~EvolutionaryAlgorithm() {
}
