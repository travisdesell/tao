/*
 * Copyright 2012, 2009 Travis Desell and the University of North Dakota.
 *
 * This file is part of the Toolkit for Asynchronous Optimization (TAO).
 *
 * TAO is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TAO is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TAO.  If not, see <http://www.gnu.org/licenses/>.
 * */

#include <iostream>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include <stdint.h>

#include "asynchronous_algorithms/evolutionary_algorithm.hxx"

#include "util/recombination.hxx"

#include "undvc_common/arguments.hxx"

#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"


using namespace std;

using boost::variate_generator;
using boost::mt19937;
using boost::uniform_real;

EvolutionaryAlgorithm::EvolutionaryAlgorithm() {
    random_number_generator = NULL;
    log_file = NULL;
}

void
EvolutionaryAlgorithm::set_log_file(ofstream *log_file) {
    this->log_file = log_file;
}

void
EvolutionaryAlgorithm::initialize_rng() {
    random_number_generator = new variate_generator< mt19937, uniform_real<> >( mt19937( time(0)), uniform_real<>(0.0, 1.0));
}


void
EvolutionaryAlgorithm::initialize() {
    Recombination::check_bounds(min_bound, max_bound);
    number_parameters = min_bound.size();

    current_iteration = 0;
    individuals_created = 0;
    individuals_reported = 0;

    maximum_iterations = 0;
    maximum_created = 0;
    maximum_reported = 0;

    random_number_generator = new variate_generator< mt19937, uniform_real<> >( mt19937( time(0)), uniform_real<>(0.0, 1.0));
    log_file = NULL;
}

void
EvolutionaryAlgorithm::parse_arguments(const vector<string> &arguments) {
    if (!get_argument(arguments, "--population_size", false, population_size)) {
        cerr << "Argument '--population_size' not specified, using default of 200." << endl;
        population_size = 200;
    }

    if (!get_argument(arguments, "--maximum_iterations", false, maximum_iterations)) {
        cerr << "Argument '--maximum_iterations' not specified, could run forever. Hit control-C to quit." << endl;
    }

    if (!get_argument(arguments, "--maximum_created", false, maximum_created)) {
        cerr << "Argument '--maximum_created' not specified, could run forever. Hit control-C to quit." << endl;
    }

    if (!get_argument(arguments, "--maximum_reported", false, maximum_reported)) {
        cerr << "Argument '--maximum_reported' not specified, could run forever. Hit control-C to quit." << endl;
    }

    wrap_radians = argument_exists(arguments, "wrap_radians");
    if (!wrap_radians) {
        cerr << "Argument '--wrap_radians' not found, parameters with a min bound of -2pi and a max bound of 2pi will not wrap around the bounds." << endl;
    }
}

EvolutionaryAlgorithm::EvolutionaryAlgorithm( const vector<string> &arguments
                                            ) throw (string) {

    get_argument_vector<double>(arguments, "--min_bound", true, min_bound);
    get_argument_vector<double>(arguments, "--min_bound", true, max_bound);

    initialize();
    parse_arguments(arguments);
    seeds = vector<uint32_t>(population_size, 0);
}

EvolutionaryAlgorithm::EvolutionaryAlgorithm( const vector<double> &min_bound,      /* min bound is copied into the search */
                                              const vector<double> &max_bound,      /* max bound is copied into the search */
                                              const vector<string> &arguments       /* initialize the DE from command line arguments */
                                            ) throw (string) {

    this->min_bound = vector<double>(min_bound);
    this->max_bound = vector<double>(max_bound);

    initialize();
    parse_arguments(arguments);
    seeds = vector<uint32_t>(population_size, 0);
}

EvolutionaryAlgorithm::EvolutionaryAlgorithm( const vector<double> &min_bound,      /* min bound is copied into the search */
                                              const vector<double> &max_bound,      /* max bound is copied into the search */
                                              const uint32_t population_size,
                                              const uint32_t maximum_iterations     /* default value is 0, which means no termination */
                                            ) throw (string) {

    this->population_size = population_size;
    this->maximum_iterations = maximum_iterations;

    this->min_bound = vector<double>(min_bound);
    this->max_bound = vector<double>(max_bound);

    initialize();
    seeds = vector<uint32_t>(population_size, 0);
}

EvolutionaryAlgorithm::EvolutionaryAlgorithm( const vector<double> &min_bound,      /* min bound is copied into the search */
                                              const vector<double> &max_bound,      /* max bound is copied into the search */
                                              const uint32_t population_size,
                                              const uint32_t maximum_created,       /* default value is 0 */
                                              const uint32_t maximum_reported       /* default value is 0 */
                                            ) throw (string) {

    this->population_size = population_size;
    this->maximum_created = maximum_created;
    this->maximum_reported = maximum_reported;

    this->min_bound = vector<double>(min_bound);
    this->max_bound = vector<double>(max_bound);

    initialize();
    seeds = vector<uint32_t>(population_size, 0);
}

EvolutionaryAlgorithm::~EvolutionaryAlgorithm() {
    if (random_number_generator != NULL) delete random_number_generator;
    if (log_file != NULL) {
        cerr << "DELETING LOG FILE!" << endl;
        delete log_file;
        cerr << "DELETED LOG FILE!" << endl;
    }
}
