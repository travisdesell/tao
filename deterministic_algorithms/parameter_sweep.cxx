#include <vector>
#include <limits>

#include "stdint.h"

#include "parameter_sweep.hxx"
#include "vector_io.hxx"

using namespace std;

void parameter_sweep(const std::vector<double> &min_bound, const std::vector<double> &max_bound, const std::vector<double> &step_size, double (*objective_function)(const std::vector<double> &)) {
    vector<double> parameters(min_bound);

    //TODO: would be cool to have queue of the best found parameters (of a user specified size) to print out at the end.

    cout.precision(15);

    double best_fitness = -numeric_limits<double>::max();
    double current_fitness;

    uint64_t iteration = 0;
    uint32_t current;
    while (parameters[parameters.size() - 1] < max_bound[parameters.size() - 1]) {
        current_fitness = objective_function(parameters);

        if (current_fitness >= best_fitness) {
            cout << iteration << " -- " << vector_to_string(parameters) << " -- " << current_fitness << endl;
            best_fitness = current_fitness;
        }

        current = 0;
        while (current < parameters.size()) {
            parameters[current] += step_size[current];

            if (parameters[current] <= max_bound[current]) {
                break;
            }

            parameters[current] = min_bound[current];
            current++;
        }

        iteration++;
    }

    cout << "total evaluations: " << iteration << endl;

}

