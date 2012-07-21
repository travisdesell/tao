#include <iostream>
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

#include "synchronous_algorithms/synchronous_gradient_descent.hxx"
#include "synchronous_algorithms/gradient.hxx"
#include "synchronous_algorithms/line_search.hxx"

#include "undvc_common/vector_io.hxx"                                                             
#include "undvc_common/arguments.hxx"                                                             

using std::vector;

void synchronous_gradient_descent(vector<string> arguments, double (*objective_function)(const std::vector<double> &), const vector<double> &starting_point, const vector<double> &step_size, LineSearch &line_search) {
    uint32_t max_iterations = 0;
    if ( !get_argument(arguments, "--max_iterations", false, max_iterations) ) {
        cerr << "Argument '--max_iterations <i>' not found, synchronous conjugate gradient descent could potentially run forever." << endl;
    }

    double min_improvement = 1e-5;
    if ( !get_argument(arguments, "--min_improvement", false, min_improvement) ) {
        cerr << "Argument ''--min_improvement <f>' not found, using default of " << min_improvement << endl;
    }

    vector<double> point(starting_point);
    vector<double> new_point(starting_point);
    vector<double> gradient(starting_point.size(), 0.0);

    double current_fitness = objective_function(point);
    double previous_fitness = current_fitness;

    for (uint32_t i = 0; max_iterations == 0 || i < max_iterations; i++) {
        cout << "iteration " << i << " -- fitness : [point] -- " << current_fitness << " : " << vector_to_string(point) << endl;

        get_gradient(objective_function, point, step_size, gradient);

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
            cout << "Search terminating because (current fitness (" << current_fitness << ") - previous fitness (" << previous_fitness << ") == " << fabs(current_fitness - previous_fitness) << ") < minimum improvement (" << min_improvement << ")" << endl;
            break;
        }
        previous_fitness = current_fitness;
    }
}

void synchronous_gradient_descent(vector<string> arguments, double (*objective_function)(const std::vector<double> &)) {
    vector<double> starting_point;
    vector<double> step_size;

    get_argument_vector<double>(arguments, "--starting_point", true, starting_point);
    get_argument_vector<double>(arguments, "--step_size", true, step_size);

    LineSearch line_search(objective_function, arguments);
    synchronous_gradient_descent(arguments, objective_function, starting_point, step_size, line_search);
}

void synchronous_gradient_descent(vector<string> arguments, double (*objective_function)(const std::vector<double> &), const vector<double> &starting_point, const vector<double> &step_size) {
    LineSearch line_search(objective_function, arguments);
    synchronous_gradient_descent(arguments, objective_function, starting_point, step_size, line_search);
}

void synchronous_gradient_descent(vector<string> arguments, double (*objective_function)(const std::vector<double> &), const vector<double> &min_bound, const vector<double> &max_bound, const vector<double> &starting_point, const vector<double> &step_size) {
    LineSearch line_search(objective_function, min_bound, max_bound, arguments);
    synchronous_gradient_descent(arguments, objective_function, starting_point, step_size, line_search);
}



void synchronous_conjugate_gradient_descent(vector<string> arguments, double (*objective_function)(const std::vector<double> &), const vector<double> &starting_point, const vector<double> &step_size, LineSearch &line_search) {
    uint32_t max_iterations = 0;
    if ( !get_argument(arguments, "--max_iterations", false, max_iterations) ) {
        cerr << "Argument '--max_iterations <i>' not found, synchronous conjugate gradient descent could potentially run forever." << endl;
    }

    uint32_t reset, reset_count = 0;
    get_argument(arguments, "--cgd_reset", true, reset);

    double min_improvement = 1e-5;
    if ( !get_argument(arguments, "--min_improvement", false, min_improvement) ) {
        cerr << "Argument ''--min_improvement <f>' not found, using default of " << min_improvement << endl;
    }

    vector<double> point(starting_point);
    vector<double> new_point(starting_point);
    vector<double> previous_gradient(starting_point.size(), 0.0);
    vector<double> previous_direction(starting_point.size(), 0.0);
    vector<double> direction(starting_point.size(), 0.0);
    vector<double> gradient(starting_point.size(), 0.0);

    double current_fitness = objective_function(point);
    double previous_fitness = current_fitness;

    double bet, betdiv;
    for (uint32_t i = 0; max_iterations == 0 || i < max_iterations; i++) {
        cout << "iteration " << i << " -- fitness : [point] -- " << current_fitness << " : " << vector_to_string(point) << endl;

        get_gradient(objective_function, point, step_size, gradient);

        if (i > 0 && reset != 0) {
            // bet = g_pres' * (g_pres - g_prev) / (g_prev' * g_prev);
            bet = 0;
            betdiv = 0;
            for (uint32_t j = 0; j < point.size(); j++) {
                bet += (gradient[j] - previous_gradient[j]) * gradient[j];
                betdiv += previous_gradient[j] * previous_gradient[j];
            }
            bet /= betdiv;

            // dpres = -g_pres + bet * d_prev;
            for (uint32_t j = 0; j < point.size(); j++) {
                direction[j] = gradient[j] + bet * previous_direction[j];
            }
        } else {
            direction.assign(gradient.begin(), gradient.end());
        }
        previous_direction.assign(direction.begin(), direction.end());
        previous_gradient.assign(gradient.begin(), gradient.end());

        cout << "\tconjugate direction: " << vector_to_string(direction) << endl;

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

        reset++;
        if (reset == reset_count) reset = 0;

        if (fabs(current_fitness - previous_fitness) < min_improvement) {
            cout << "Search terminating because (current fitness (" << current_fitness << ") - previous fitness (" << previous_fitness << ") == " << fabs(current_fitness - previous_fitness) << ") < minimum improvement (" << min_improvement << ")" << endl;
            break;
        }
        previous_fitness = current_fitness;
    }
}

void synchronous_conjugate_gradient_descent(vector<string> arguments, double (*objective_function)(const std::vector<double> &)) {
    vector<double> starting_point;
    vector<double> step_size;

    get_argument_vector<double>(arguments, "--starting_point", true, starting_point);
    get_argument_vector<double>(arguments, "--step_size", true, step_size);

    LineSearch lien_search(objective_function, arguments);
    synchronous_conjugate_gradient_descent(arguments, objective_function, starting_point, step_size);
}

void synchronous_conjugate_gradient_descent(vector<string> arguments, double (*objective_function)(const std::vector<double> &), const vector<double> &starting_point, const vector<double> &step_size) {
    LineSearch line_search(objective_function, arguments);
    synchronous_conjugate_gradient_descent(arguments, objective_function, starting_point, step_size, line_search);
}

void synchronous_conjugate_gradient_descent(vector<string> arguments, double (*objective_function)(const std::vector<double> &), const vector<double> &min_bound, const vector<double> &max_bound, const vector<double> &starting_point, const vector<double> &step_size) {
    LineSearch line_search(objective_function, min_bound, max_bound, arguments);
    synchronous_conjugate_gradient_descent(arguments, objective_function, starting_point, step_size, line_search);
}


