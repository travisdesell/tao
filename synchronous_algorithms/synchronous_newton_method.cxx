#include <vector>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "stdint.h"

#include "synchronous_algorithms/gradient.hxx"
#include "synchronous_algorithms/hessian.hxx"
#include "synchronous_algorithms/newton_method.hxx"
#include "synchronous_algorithms/line_search.hxx"

#include "undvc_common/vector_io.hxx"
#include "undvc_common/arguments.hxx"

using namespace std;

void synchronous_newton_method(vector<string> arguments, double (*objective_function)(const std::vector<double> &)) {
    vector<double> starting_point;
    vector<double> step_size;

    get_argument_vector<double>(arguments, "--starting_point", true, starting_point);
    get_argument_vector<double>(arguments, "--step_size", true, step_size);

    synchronous_newton_method(arguments, objective_function, starting_point, step_size);
}

void synchronous_newton_method(vector<string> arguments, double (*objective_function)(const std::vector<double> &), const vector<double> &starting_point, const vector<double> &step_size) {
	double current_fitness;
	uint32_t iterations, retval, evaluations;

    get_argument(arguments, "--iterations", true, iterations);

    vector<double> point(starting_point);
    vector<double> new_point(point.size(), 0.0);
    vector<double> direction(point.size(), 0.0);
    vector<double> gradient(point.size(), 0.0);
    vector< vector<double> > hessian( point.size(), vector<double>(point.size(), 0.0));

	current_fitness = objective_function(point);

    cout.precision(15);

    LineSearch line_search();

	for (uint32_t i = 0; i < iterations; i++) {
        cout << "iteration " << i << " -- fitness : [point] -- " << current_fitness << " : " << vector_to_string(point) << endl;

        cout << "\tcalculating gradient." << endl;
		get_gradient(number_parameters, point, range, gradient);

        cout << "\tcalculating hessian." << endl;
		get_hessian(number_parameters, point, range, hessian);

        cout << "\tcalculating direction." << endl;
		newton_step(number_parameters, hessian, gradient, direction);
		for (uint32_t j = 0; j < direction.size(); j++) direction[j] = -direction[j];

        cout << "\tdirection: " << vector_to_string(direction) << endl;

        line_search.go(point, current_fitness, step, number_parameters, new_point, &current_fitness, &evaluations);

        cout << "\tnew point: " << vector_to_string(new_point) << endl;
		cout << "\tline search took: " << evaluations << " evaluations for new fitness: " << current_fitness << ", with result: [" << line_search.get_status() << "]" << endl;

        if ( !line_search.made_progress() ) break;

        point.assign(new_point);
	}
}
