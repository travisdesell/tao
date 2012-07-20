#include <vector>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "stdint.h"

#include "synchronous_algorithms/synchronous_newton_method.hxx"
#include "synchronous_algorithms/gradient.hxx"
#include "synchronous_algorithms/hessian.hxx"
#include "synchronous_algorithms/newton_step.hxx"
#include "synchronous_algorithms/line_search.hxx"

#include "undvc_common/vector_io.hxx"
#include "undvc_common/arguments.hxx"

using namespace std;


void synchronous_newton_method(vector<string> arguments, double (*objective_function)(const std::vector<double> &), const vector<double> &starting_point, const vector<double> &step_size, LineSearch &line_search) {
	uint32_t iterations;
    get_argument(arguments, "--iterations", true, iterations);

    vector<double> point(starting_point);
    vector<double> new_point(point.size(), 0.0);
    vector<double> direction(point.size(), 0.0);
    vector<double> gradient(point.size(), 0.0);
    vector< vector<double> > hessian( point.size(), vector<double>(point.size(), 0.0));

	double current_fitness = objective_function(point);

    cout.precision(15);

	for (uint32_t i = 0; i < iterations; i++) {
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
                break; //dont quit for out of bounds errors
            }

            delete lse;
        }

        cout << "\tnew fitness : [point] -- " << current_fitness << " : " << vector_to_string(new_point) << endl;

        point.assign(new_point.begin(), new_point.end());
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


