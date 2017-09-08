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

#include <vector>
#include <limits>

#include "stdint.h"

#include "parameter_sweep.hxx"
#include "util/vector_io.hxx"

using namespace std;

void parameter_sweep(const std::vector<double> &min_bound, const std::vector<double> &max_bound, const std::vector<double> &step_size, double (*objective_function)(const std::vector<double> &)) {
    vector<double> parameters(min_bound);

    //TODO: would be cool to have queue of the best found parameters (of a user specified size) to print out at the end.

    cout.precision(15);

    double best_fitness = -numeric_limits<double>::max();
    double current_fitness;

    for (uint32_t i = 0; i < max_bound.size(); i++) {
        cout << "max_bound[" << i << "]: " << max_bound[i] << ", min_bound[" << i << "]: " << min_bound[i] << ", step_size[" << i << "]: " << step_size[i] << ", steps: " << ((max_bound[i] - min_bound[i]) / step_size[i]) + 1 << endl;
    }

    uint64_t expected_evaluations = ((max_bound[0] - min_bound[0]) / step_size[0]) + 1;
    for (uint32_t i = 1; i < max_bound.size(); i++) {
        expected_evaluations *= ((max_bound[i] - min_bound[i]) / step_size[i]) + 1;
    }

    cout << "expected evaluations: " << expected_evaluations << endl;

    uint64_t iteration = 0;
    while (iteration < expected_evaluations) {
        current_fitness = objective_function(parameters);

        if (current_fitness >= best_fitness) {
            cout << iteration << " -- " << vector_to_string(parameters) << " -- " << current_fitness << endl;
            best_fitness = current_fitness;
        }

        parameters[0] += step_size[0];
        for (uint32_t i = 1; i < parameters.size(); i++) {
            if (parameters[i - 1] > max_bound[i - 1]) {
                parameters[i - 1] = min_bound[i - 1];
                parameters[i] += step_size[i];
            }
        }

        iteration++;
    }

    cout << "total    evaluations: " << iteration << endl;
}

