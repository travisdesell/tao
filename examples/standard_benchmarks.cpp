#include <cmath>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <cfloat>
#include <string>
#include <stdint.h>

#include "../evolutionary_algorithms/particle_swarm.hxx"
#include "../evolutionary_algorithms/differential_evolution.hxx"

//from undvc_common
#include "arguments.hxx"

/**
 *  Note that M_PI is C's declaration for pi
 */

using namespace std;

/**
 *  Define the sphere, ackely, griewank, rastrigin and rosenbrock 
 *  objective functions.
 */
double sphere(const vector<double> &x) {
    uint32_t i;
    double sum;

    sum = 0.0;
    for (i = 0; i < x.size(); i++) {
        sum += x[i] * x[i];
    }
    return -sum;
}

double ackley(const vector<double> &x) {
    uint32_t i;
    double sum1, sum2;

    sum1 = 0.0;
    for (i = 0; i < x.size(); i++) {
        sum1 += x[i] * x[i];
    }
    sum1 /= x.size();
    sum1 = -0.2 * sqrt(sum1);

    sum2 = 0.0;
    for (i = 0; i < x.size(); i++) {
        sum2 += cos(2 * M_PI * x[i]);
    }
    sum2 /= x.size();

    return -(20 + M_E - (20 * (exp(sum1)) - exp(sum2)) );
}

double griewank(const vector<double> &x) {
    uint32_t i;
    double sum1, sum2;

    sum1 = 0.0;
    for (i = 0; i < x.size(); i++) {
        sum1 += x[i] * x[i];
    }
    sum1 /= 4000;

    sum2 = cos( x[0] / sqrt(1) );
    for (i = 1; i < x.size(); i++) {
        sum2 *= cos( x[i] / sqrt(i+1) );
    }

    return -(sum1 - sum2 + 1);
}

double rastrigin(const vector<double> &x) {
    uint32_t i;
    double sum;

    sum = 0.0;
    for (i = 0; i < x.size(); i++) {
        sum += (x[i] * x[i]) - (10 * cos(2 * M_PI * x[i])) + 10;
    }
    return -sum;
}

double rosenbrock(const vector<double> &x) {
    uint32_t i;
    double sum, tmp;

    sum = 0.0;
    for (i = 0; i < x.size() - 1; i++) {
        tmp = (x[i + 1] - (x[i] * x[i]));
        sum += (100 * tmp * tmp) + ((x[i] - 1) * (x[i] - 1));
    }

    return -sum;
}

/**
 *  Define a type for our objective function so we
 *  can pick which one to use and then the rest of
 *  the code will be the same.
 *
 *  Note that all the objective functions return a
 *  double, and take a double* and uint32_t as arguments.
 */
typedef double (*objective_function)(const vector<double> &);

int main(uint32_t argc /* number of command line arguments */, char **argv /* command line argumens */ ) {
    vector<string> arguments(argv, argv + argc);

    //assign the objective function variable to the objective function we're going to use
    string objective_function_name;
    get_argument(arguments, "--objective_function", true, objective_function_name);

    objective_function f = NULL;
    //compare returns 0 if the two strings are the same
    if (objective_function_name.compare("sphere") == 0)             f = sphere;
    else if (objective_function_name.compare("ackley") == 0)        f = ackley;
    else if (objective_function_name.compare("griewank") == 0)      f = griewank;
    else if (objective_function_name.compare("rastrigin") == 0)     f = rastrigin;
    else if (objective_function_name.compare("rosenbrock") == 0)    f = rosenbrock;
    else {
        fprintf(stderr, "Improperly specified objective function: '%s'\n", objective_function_name.c_str());
        fprintf(stderr, "Possibilities are:\n");
        fprintf(stderr, "    sphere\n");
        fprintf(stderr, "    ackley\n");
        fprintf(stderr, "    griewank\n");
        fprintf(stderr, "    rastrigin\n");
        fprintf(stderr, "    rosenbrock\n");
        exit(0);
    }

    /**
     *  Initialize the arrays for the minimum and maximum bounds of the search space.
     *  These are different for the different objective functions.
     */
    uint32_t number_of_parameters;
    get_argument(arguments, "--n_parameters", true, number_of_parameters);
    vector<double> min_bound(number_of_parameters, 0);
    vector<double> max_bound(number_of_parameters, 0);

    for (uint32_t i = 0; i < number_of_parameters; i++) {        //arrays go from 0 to size - 1 (not 1 to size)
        if (objective_function_name.compare("sphere") == 0) {
            min_bound[i] = -100;
            max_bound[i] = 100;
        } else if (objective_function_name.compare("ackley") == 0) {
            min_bound[i] = -32;
            max_bound[i] = 32;
        } else if (objective_function_name.compare("griewank") == 0) {
            min_bound[i] = -600;
            max_bound[i] = 600;
        } else if (objective_function_name.compare("rastrigin") == 0) {
            min_bound[i] = -5.12;
            max_bound[i] = 5.12;
        } else if (objective_function_name.compare("rosenbrock") == 0) {
            min_bound[i] = -5;
            min_bound[i] = 10;
        }
    }

    string search_type;
    get_argument(arguments, "--search_type", true, search_type);
    if (search_type.compare("ps") == 0) {
        ParticleSwarm ps(min_bound, max_bound, arguments);
        ps.iterate(f);

    } else if (search_type.compare("de") == 0) {
        DifferentialEvolution de(min_bound, max_bound, arguments);
        de.iterate(f);

    } else {
        fprintf(stderr, "Improperly specified search type: '%s'\n", search_type.c_str());
        fprintf(stderr, "Possibilities are:\n");
        fprintf(stderr, "    de     -       differential evolution\n");
        fprintf(stderr, "    ps     -       particle swarm optimization\n");
        exit(0);
    }

    return 0;
}
