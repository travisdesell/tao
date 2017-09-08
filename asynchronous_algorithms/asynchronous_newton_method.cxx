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

#include <string>
#include <vector>
#include <limits>
#include <iostream>
#include <iomanip>

#include "asynchronous_algorithms/asynchronous_newton_method.hxx"

#include "util/arguments.hxx"
#include "util/recombination.hxx"
#include "util/statistics.hxx"
#include "util/newton_step.hxx"
#include "util/hessian.hxx"
#include "util/vector_io.hxx"


using namespace std;


AsynchronousNewtonMethod::~AsynchronousNewtonMethod() {
}

AsynchronousNewtonMethod::AsynchronousNewtonMethod() {
}

AsynchronousNewtonMethod::AsynchronousNewtonMethod(
                                    const vector<string> &arguments
                                ) throw (string) {
    parse_arguments(arguments);
    initialize();
}

void
AsynchronousNewtonMethod::initialize_rng() {
    random_number_generator = mt19937(time(0));
    random_0_1 = uniform_real_distribution<double>(0, 1.0);
}

void
AsynchronousNewtonMethod::pre_initialize() {
    initialize_rng();

    maximum_iterations_defined = false;
    line_search_min_defined = false;
    line_search_max_defined = false;
    extra_workunits_defined = false;
    minimum_line_search_individuals_defined = false;
    minimum_regression_individuals_defined = false;
    regression_radius_defined = false;
    center_defined = false;
    min_bound_defined = false;
    max_bound_defined = false;
    max_failed_improvements_defined = false;
}

AsynchronousNewtonMethod::AsynchronousNewtonMethod(
                                    const vector<double> &min_bound,
                                    const vector<double> &max_bound,
                                    const vector<double> &regression_radius,
                                    const vector<string> &arguments
                                ) throw (string) {

    pre_initialize();

    this->max_bound_defined = true;
    this->max_bound.assign(max_bound.begin(), max_bound.end());

    this->min_bound_defined = true;
    this->min_bound.assign(min_bound.begin(), min_bound.end());

    this->regression_radius_defined = true;
    this->regression_radius.assign(regression_radius.begin(), regression_radius.end());

    parse_arguments(arguments);
    initialize();
}

AsynchronousNewtonMethod::AsynchronousNewtonMethod(
                                    const vector<double> &min_bound,
                                    const vector<double> &max_bound,
                                    const vector<double> &center,
                                    const vector<double> &regression_radius,
                                    const vector<string> &arguments
                                ) throw (string) {

    pre_initialize();

    this->max_bound_defined = true;
    this->max_bound.assign(max_bound.begin(), max_bound.end());

    this->min_bound_defined = true;
    this->min_bound.assign(min_bound.begin(), min_bound.end());

    this->center_defined = true;
    this->center.assign(center.begin(), center.end());

    this->regression_radius_defined = true;
    this->regression_radius.assign(regression_radius.begin(), regression_radius.end());

    parse_arguments(arguments);
    initialize();
}

AsynchronousNewtonMethod::AsynchronousNewtonMethod(
                                    const vector<double> &min_bound,                  /* min bound is copied into the search */
                                    const vector<double> &max_bound,                  /* max bound is copied into the search */
                                    const vector<double> &center,
                                    const vector<double> &regression_radius,
                                    const uint32_t minimum_regression_individuals,
                                    const uint32_t minimum_line_search_individuals,
                                    const uint32_t maximum_iterations                 /* default value is 0 which means no termination */
                                ) throw (string) {

    pre_initialize();

    this->max_bound_defined = true;
    this->max_bound.assign(max_bound.begin(), max_bound.end());

    this->min_bound_defined = true;
    this->min_bound.assign(min_bound.begin(), min_bound.end());

    this->center_defined = true;
    this->center.assign(center.begin(), center.end());

    this->regression_radius_defined = true;
    this->regression_radius.assign(regression_radius.begin(), regression_radius.end());

    this->minimum_regression_individuals_defined = true;
    this->minimum_regression_individuals = minimum_regression_individuals;

    this->minimum_line_search_individuals_defined = true;
    this->minimum_line_search_individuals = minimum_line_search_individuals;

    this->maximum_iterations_defined = true;
    this->maximum_iterations = maximum_iterations;

    initialize();
}

void
AsynchronousNewtonMethod::parse_arguments(const vector<string> &arguments) {
    if (!min_bound_defined) {
        get_argument_vector(arguments, "--min_bound", true, min_bound);
    }

    if (!max_bound_defined) {
        get_argument_vector(arguments, "--max_bound", true, max_bound);
    }

    number_parameters = min_bound.size();

    if (!regression_radius_defined) {
        //This is the radius in which the random individuals are generated to calculate the hessian/gradient
        get_argument_vector(arguments, "--regression_radius", true, regression_radius);
    }

    if (!center_defined &&
            !get_argument_vector(arguments, "--initial_point", false, center)) {
        cerr << "Argument '--initial_point <f1, f2, .. fn>' not specified, using random starting point:" << endl;
        Recombination::random_within(min_bound, max_bound, center, random_number_generator, random_0_1);
        cerr << "\tmin_bound: " << vector_to_string(min_bound) << endl;
        cerr << "\tmax_bound: " << vector_to_string(max_bound) << endl;
        cerr << "\tpoint:     " << vector_to_string(center) << endl;
    }

    cout << "minimum_regression_individuals_defined: " << minimum_regression_individuals_defined << endl;
    cout << "minimum_line_search_individuals_defined: " << minimum_line_search_individuals_defined << endl;

    uint32_t rps_min = 4 * number_parameters * number_parameters;
    if (!minimum_regression_individuals_defined &&
            (!get_argument(arguments, "--minimum_regression_individuals", false, minimum_regression_individuals) || minimum_regression_individuals  < rps_min)) {
        cerr << "Argument '--minimum_regression_individuals <I>' not found or less than minimum, using minimum of 4 * number_parameters^2 = " << rps_min << endl;
        minimum_regression_individuals = rps_min;
    }

    if (!minimum_line_search_individuals_defined &&
            !get_argument(arguments, "--minimum_line_search_individuals", false, minimum_line_search_individuals)) {
        minimum_line_search_individuals = 500;
        cerr << "Argument '--minimum_line_search_individuals <I>' not found, using default of " << minimum_line_search_individuals << endl;
    }

    if (!line_search_min_defined &&
            !get_argument(arguments, "--line_search_min", false, line_search_min)) {
        line_search_min = -1;
        cerr << "Argument '--line_search_min <F> not found, using default of -1" << endl;
    }

    if (!line_search_max_defined &&
            !get_argument(arguments, "--line_search_max", false, line_search_max)) {
        line_search_max = 3;
        cerr << "Argument '--line_search_max <F> not found, using default of 3" << endl;
    }


    if (!maximum_iterations_defined &&
            !get_argument(arguments, "--maximum_iterations", false, maximum_iterations)) {
        cerr << "Argument '--maximum_iterations <I>' not found, using default of 0. Search will not terminate automatically." << endl;
        maximum_iterations = 0;
    }

    if (!extra_workunits_defined &&
            !get_argument(arguments, "--extra_workunits", false, extra_workunits)) {
        cerr << "Argument '--extra_worknits <I>' not found, using default of 100." << endl;
        extra_workunits = 100;
    }

    if (!max_failed_improvements_defined &&
            !get_argument(arguments, "--max_failed_improvements", false, max_failed_improvements)) {
        cerr << "Argument '--max_failed_improvements <I> not found, using default of 0. Search may not terminate automatically." << endl;
        max_failed_improvements = 0;
    }
}


void
AsynchronousNewtonMethod::initialize() {
    regression_individuals  = vector< vector<double> >(minimum_regression_individuals + extra_workunits, vector<double>(number_parameters, 0.0));
    regression_fitnesses    = vector<double>(minimum_regression_individuals + extra_workunits);
    regression_seeds        = vector<uint32_t>(minimum_regression_individuals + extra_workunits);

    line_search_individuals = vector< vector<double> >(minimum_line_search_individuals + extra_workunits, vector<double>(number_parameters, 0.0));
    line_search_fitnesses   = vector<double>(minimum_line_search_individuals + extra_workunits);
    line_search_seeds       = vector<uint32_t>(minimum_line_search_individuals + extra_workunits);

    regression_individuals_reported     = 0;
    line_search_individuals_reported    = 0;

    line_search_direction = vector<double>(number_parameters, 0.0);

    Recombination::check_bounds(min_bound, max_bound);
    Recombination::check_step(regression_radius);

    first_workunits_generated = false;

    current_iteration = 0;

    center_fitness = -std::numeric_limits<double>::max();
    failed_improvements = 0;
}

bool
AsynchronousNewtonMethod::generate_individuals(uint32_t &number_individuals, uint32_t &iteration, vector< vector<double> > &parameters, vector<uint32_t> &seeds) throw (string) {
    if (!generate_individuals(number_individuals, iteration, parameters)) return false;

    seeds.resize(number_individuals);
    for (uint32_t i = 0; i < number_individuals; i++) {
        seeds[i] = (random_0_1(random_number_generator) * numeric_limits<uint32_t>::max()) / 10.0;    //for some reason uint32_t is too large for milkyway_nbody
    }

    return true;
}

//returns true if it generates individuals
bool
AsynchronousNewtonMethod::generate_individuals(uint32_t &number_individuals, uint32_t &iteration, vector< vector<double> > &parameters) throw (string) {
    if (!first_workunits_generated) {

        iteration = this->current_iteration;

        number_individuals = minimum_regression_individuals + extra_workunits;

//        cout << "generating first workunits (" << number_individuals << ")" << endl;

        parameters.resize(number_individuals, vector<double>(number_parameters));

        for (uint32_t i = 0; i < number_individuals; i++) {
            Recombination::random_around(center, regression_radius, parameters[i], random_number_generator, random_0_1);
            Recombination::bound_parameters(min_bound, max_bound, parameters[i]);
//            cout << "generated parameters: " << vector_to_string(parameters[i]) << endl;
        }
        regression_individuals_reported = 0;
        //resize these so they can fit all the generated workunits
        regression_individuals.resize(minimum_regression_individuals + extra_workunits, vector<double>(number_parameters));  
        regression_fitnesses.resize(minimum_regression_individuals + extra_workunits, 0.0);

        first_workunits_generated = true;

        return true;

    } else if (this->current_iteration % 2 == 0 && regression_individuals_reported >= minimum_regression_individuals) {
        //even iterations calculate gradient/hessian
        //this iteration has finished, so set the direction
        regression_individuals.resize(regression_individuals_reported);  //need to resize these so the randomized_hessian function works right
        regression_fitnesses.resize(regression_individuals_reported);

        vector< vector<double> > hessian;
        vector<double> gradient;
        try {
            randomized_hessian(regression_individuals, center, regression_fitnesses, hessian, gradient);
        } catch (string err_msg) {
            cout << "randomied hessian threw string error: " << endl;
            cout << "\t" << err_msg << endl;
            exit(0);
        }

        try {
            newton_step(hessian, gradient, line_search_direction);
        } catch (string err_msg) {
            cout << "newton step threw string error: " << endl;
            cout << "\t" << err_msg << endl;
            exit(0);
        }

        cout << "center:                " << vector_to_string(center) << endl;
        cout << "line_search_direction: " << vector_to_string(line_search_direction) << endl;

        this->current_iteration++;
        iteration = this->current_iteration;
        
        number_individuals = minimum_line_search_individuals + extra_workunits;
        parameters.resize(number_individuals, vector<double>(number_parameters));

//        cout << "generating line search workunits (" << number_individuals << ")" << endl;

        for (uint32_t i = 0; i < number_individuals; i++) {
            Recombination::random_along(center, line_search_direction, line_search_min, line_search_max, parameters[i], random_number_generator, random_0_1);
            Recombination::bound_parameters(min_bound, max_bound, parameters[i]);
//            cout << "generated parameters: " << vector_to_string(parameters[i]) << endl;
        }
        line_search_individuals_reported = 0;
        line_search_individuals.resize(minimum_line_search_individuals + extra_workunits, vector<double>(number_parameters));

        return true;
    } else if (line_search_individuals_reported >= minimum_line_search_individuals) {
        //odd iterations do a line search
        //this iteration has finished so set the new center 
        line_search_individuals.resize(line_search_individuals_reported);

        uint32_t best = 0;
        double best_fitness = line_search_fitnesses[0];
        for (uint32_t i = 1; i < line_search_fitnesses.size(); i++) {
            if (best_fitness < line_search_fitnesses[i]) best = i;
        }
        
        if (best_fitness > center_fitness) {
            center.assign(line_search_individuals[best].begin(), line_search_individuals[best].end());
            center_fitness = best_fitness;
            
            failed_improvements = 0;
//            cout << "best individual:          " << best << endl;
            cout << "best individual found:    " << vector_to_string(line_search_individuals[best]) << endl;
            cout << "best individual fitness:  " << best_fitness << endl;
        } else {
            failed_improvements++;
//            cout << "best individual:          " << best << endl;
            cout << "best individual found:    " << vector_to_string(line_search_individuals[best]) << endl;
            cout << "best individual fitness:  " << best_fitness << endl;
            cout << "no improvement:           " << failed_improvements << endl;
            cout << "using previous center:    " << vector_to_string(center) << endl;
            cout << "preveious center fitness: " << center_fitness << endl;
        }

        this->current_iteration++;
        iteration = this->current_iteration;
        
        number_individuals = minimum_regression_individuals + extra_workunits;
        parameters.resize(number_individuals, vector<double>(number_parameters));

//        cout << "generating regression workunits (" << number_individuals << ")" << endl;

        for (uint32_t i = 0; i < number_individuals; i++) {
            Recombination::random_around(center, regression_radius, parameters[i], random_number_generator, random_0_1);
            Recombination::bound_parameters(min_bound, max_bound, parameters[i]);
//            cout << "generated parameters: " << vector_to_string(parameters[i]) << endl;
        }
        regression_individuals_reported = 0;
        //resize these so they can fit all the generated workunits
        regression_individuals.resize(minimum_regression_individuals + extra_workunits, vector<double>(number_parameters));  
        regression_fitnesses.resize(minimum_regression_individuals + extra_workunits, 0.0);

        return true;
    }

    return false;
}

bool
AsynchronousNewtonMethod::insert_individual(uint32_t iteration, const vector<double> &parameters, double fitness, uint32_t seed) throw (string) {
    if (!insert_individual(iteration, parameters, fitness)) return false;

    if (iteration % 2 == 0) {
        //even iterations calculate a hessian/gradient
        regression_seeds[regression_individuals_reported - 1] = seed;
    } else {
        //odd iterations do a line search
        line_search_seeds[line_search_individuals_reported - 1] = seed;
    }

    return true;
}

bool
AsynchronousNewtonMethod::insert_individual(uint32_t iteration, const vector<double> &parameters, double fitness) throw (string) {
    bool modified = false;

    if (iteration == this->current_iteration) {
        if (iteration % 2 == 0 && regression_individuals_reported < minimum_regression_individuals + extra_workunits) {
            //even iterations calculate a hessian/gradient

//            cout << "setting [regression]  " << regression_individuals_reported << " -- fitness: " << fitness << " -- parameters " << vector_to_string(parameters) << endl;
            regression_individuals[regression_individuals_reported] = parameters;
            regression_fitnesses[regression_individuals_reported] = fitness;
            regression_individuals_reported++;

            modified = true;
        } else if (line_search_individuals_reported < minimum_line_search_individuals + extra_workunits) {
            //odd iterations do a line search
            //
//            cout << "setting [line search] " << line_search_individuals_reported << " -- fitness: " << fitness << " -- parameters " << vector_to_string(parameters) << endl;
            line_search_individuals[line_search_individuals_reported] = parameters;
            line_search_fitnesses[line_search_individuals_reported] = fitness;
            line_search_individuals_reported++;

            modified = true;
        }
    }

    return modified;
}

void
AsynchronousNewtonMethod::iterate(double (*objective_function)(const vector<double> &)) throw (string) {
    cout << "Initialized asynchronous newton method." << endl;
    cout << "   maximum_iterations: " << maximum_iterations << endl;
    cout << "   minimum_line_search_individuals: " << minimum_line_search_individuals << endl;
    cout << "   minimum_regression_individuals: " << minimum_regression_individuals << endl;
    cout << "   extra_workunits: " << extra_workunits << endl;
    cout << "   min_bound:  " << vector_to_string(min_bound) << endl;
    cout << "   max_bound:  " << vector_to_string(max_bound) << endl;
    cout << "   center:  " << vector_to_string(center) << endl;
    cout << "   regression_radius:  " << vector_to_string(regression_radius) << endl;

    uint32_t individuals_iteration;
    uint32_t number_individuals;
    vector< vector<double> > individuals;
    vector<double> fitnesses;

    if (current_iteration > 0) {
        //This is a hack for restarting ANMs using a database
        if (current_iteration % 2 == 1) current_iteration--;
        regression_individuals_reported = minimum_regression_individuals;
    }

    while (maximum_iterations == 0 || current_iteration < maximum_iterations) {
        if ( !generate_individuals(number_individuals, individuals_iteration, individuals) ) {
            cerr << "generated individuals didn't generate any individuals." << endl;
            break;
        }
        fitnesses.resize(individuals.size());

        if (max_failed_improvements > 0 && failed_improvements >= max_failed_improvements) break;

        for (uint32_t i = 0; i < individuals.size(); i++) {
            fitnesses[i] = objective_function(individuals[i]);

            insert_individual(individuals_iteration, individuals[i], fitnesses[i]);
        }
    }
}

void
AsynchronousNewtonMethod::iterate(double (*objective_function)(const vector<double> &, const uint32_t)) throw (string) {
    cout << "Initialized asynchronous newton method." << endl;
    cout << "   maximum_iterations: " << maximum_iterations << endl;
    cout << "   minimum_line_search_individuals: " << minimum_line_search_individuals << endl;
    cout << "   minimum_regression_individuals: " << minimum_regression_individuals << endl;
    cout << "   extra_workunits: " << extra_workunits << endl;
    cout << "   min_bound:  " << vector_to_string(min_bound) << endl;
    cout << "   max_bound:  " << vector_to_string(max_bound) << endl;
    cout << "   center:  " << vector_to_string(center) << endl;
    cout << "   regression_radius:  " << vector_to_string(regression_radius) << endl;

    uint32_t individuals_iteration;
    uint32_t number_individuals;
    vector< vector<double> > individuals;
    vector<double> fitnesses;
    vector<uint32_t> seeds;

    if (current_iteration > 0) {
        //This is a hack for restarting ANMs using a database
        if (current_iteration % 2 == 1) current_iteration--;
        regression_individuals_reported = minimum_regression_individuals;
    }

    while (maximum_iterations == 0 || current_iteration < maximum_iterations) {
        if ( !generate_individuals(number_individuals, individuals_iteration, individuals, seeds) ) {
            cerr << "generated individuals didn't generate any individuals." << endl;
            break;
        }
        fitnesses.resize(individuals.size());

        if (max_failed_improvements > 0 && failed_improvements >= max_failed_improvements) break;

        for (uint32_t i = 0; i < individuals.size(); i++) {
            fitnesses[i] = objective_function(individuals[i], seeds[i]);

            insert_individual(individuals_iteration, individuals[i], fitnesses[i], seeds[i]);
        }
    }
}
