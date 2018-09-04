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

    sum2 = cos( x[0] / sqrt(1.0) );
    for (i = 1; i < x.size(); i++) {
        sum2 *= cos( x[i] / sqrt((double)i+1) );
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
