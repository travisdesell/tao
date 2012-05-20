#ifndef TAO_BENCHMARKS_H
#define TAO_BENCHMARKS_H

#include <cmath>
#include <vector>


using namespace std;

/**
 *  Note that M_PI is C's declaration for pi
 */

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


#endif
