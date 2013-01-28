#include <vector>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>

#define _USE_MATH_DEFINES
#include <math.h>

#include "stdint.h"

#include "synchronous_algorithms/synchronous_newton_method.hxx"
#include "synchronous_algorithms/gradient.hxx"
#include "util/hessian.hxx"
#include "util/newton_step.hxx"
#include "synchronous_algorithms/line_search.hxx"

#include "undvc_common/vector_io.hxx"
#include "undvc_common/arguments.hxx"

using namespace std;


void synchronous_newton_method(vector<string> arguments, double (*objective_function)(const std::vector<double> &), const vector<double> &starting_point, const vector<double> &step_size, LineSearch &line_search) {
	uint32_t max_iterations = 0;
    if ( !get_argument(arguments, "--max_iterations", false, max_iterations) ) {
        cerr << "Argument '--max_iterations <i>' not found, synchronous newton method could potentially run forever." << endl;
    }

    double min_improvement = 1e-5;
    if ( !get_argument(arguments, "--min_improvement", false, min_improvement) ) {
        cerr << "Argument ''--min_improvement <f>' not found, using default of " << min_improvement << endl;
    }


    vector<double> point(starting_point);
    vector<double> new_point(point.size(), 0.0);
    vector<double> direction(point.size(), 0.0);
    vector<double> gradient(point.size(), 0.0);
    vector< vector<double> > hessian( point.size(), vector<double>(point.size(), 0.0));

	double current_fitness = objective_function(point);
    double previous_fitness = current_fitness;

    cout.precision(15);

	for (uint32_t i = 0; max_iterations == 0 || i < max_iterations; i++) {
        cout << "iteration " << i << " -- fitness : [point] -- " << current_fitness << " : " << vector_to_string(point) << endl;

        try {
            get_gradient(objective_function, point, step_size, gradient);
            get_hessian(objective_function, point, step_size, hessian);
            newton_step(hessian, gradient, direction);
        } catch (const char *err_msg) {
            cout << "\tCalculating gradient and hessian failed with message: [" << err_msg << "]" << endl;
            break;
        }

        for (uint32_t j = 0; j < direction.size(); j++) direction[j] = -direction[j];

        cout << "\t\tdirection: " << vector_to_string(direction) << endl;

        try {
            line_search.line_search(point, current_fitness, gradient, new_point, current_fitness);
        } catch (LineSearchException *lse) {
            cout << "\tLINE SEARCH EXCEPTION: " << *lse << endl;

            if (lse->get_type() != LineSearchException::LOOP_2_OUT_OF_BOUNDS && lse->get_type() != LineSearchException::LOOP_3_OUT_OF_BOUNDS) {
                delete lse;
                break; //dont quit for out of bounds errors
            } else {
                delete lse;
            }
        }

        cout << "\tnew fitness : [point] -- " << current_fitness << " : " << vector_to_string(new_point) << endl;

        point.assign(new_point.begin(), new_point.end());

        if (fabs(current_fitness - previous_fitness) < min_improvement) {
            cout << "Search terminating because (current fitness (" << current_fitness << ") - previous fitness (" << previous_fitness << ") == " << fabs(current_fitness - previous_fitness) << ") < minimum improvement     (" << min_improvement << ")" << endl;
            break;
        }
        previous_fitness = current_fitness;
	}
}

void synchronous_newton_method(vector<string> arguments, double (*objective_function)(const std::vector<double> &)) {
    vector<double> starting_point;
    vector<double> step_size;

    get_argument_vector<double>(arguments, "--starting_point", true, starting_point);
    get_argument_vector<double>(arguments, "--step_size", true, step_size);

    LineSearch line_search(objective_function, arguments);

    synchronous_newton_method(arguments, objective_function, starting_point, step_size, line_search);
}

void synchronous_newton_method(vector<string> arguments, double (*objective_function)(const std::vector<double> &), const vector<double> &starting_point, const vector<double> &step_size) {
    LineSearch line_search(objective_function, arguments);
    synchronous_newton_method(arguments, objective_function, starting_point, step_size, line_search);
}

void synchronous_newton_method(vector<string> arguments, double (*objective_function)(const std::vector<double> &), const vector<double> &min_bound, const vector<double> &max_bound, const vector<double> &starting_point, const vector<double> &step_size) {
    LineSearch line_search(objective_function, min_bound, max_bound, arguments);
    synchronous_newton_method(arguments, objective_function, starting_point, step_size, line_search);
}


