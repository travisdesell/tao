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
#include <stdint.h>
#include <string>
#include <sstream>
#include <cstdlib>
#include <iostream>

#include <random>
using std::mt19937;
using std::uniform_real_distribution;

#include "recombination.hxx"

using std::vector;
using std::cout;
using std::endl;


/**
 *  Functions dealing with bounds.
 */
void
Recombination::bound_parameters(const vector<double> &min_bound, const vector<double> &max_bound, vector<double> &dest, bool wrap_radians) {
    for (uint32_t i = 0; i < min_bound.size(); i++) {
        if (wrap_radians && (fabs(min_bound[i] - (-2 * M_PI)) < 0.00001) && (fabs(max_bound[i] - ( 2 * M_PI)) < 0.0000001) ) {
//            cout << "\tbounding radian start: " << dest[i] << endl;

            while (dest[i] > max_bound[i]) dest[i] -= (2 * M_PI);
            while (dest[i] < min_bound[i]) dest[i] += (2 * M_PI);

//            cout << "\tbounding radian end:   " << dest[i] << endl;

        } else {
            if (dest[i] < min_bound[i]) dest[i] = min_bound[i];
            if (dest[i] > max_bound[i]) dest[i] = max_bound[i];
        }
    }
}

bool
Recombination::out_of_bounds(const vector<double> &min_bound, const vector<double> &max_bound, const vector<double> &parameters) {
    for (uint32_t i = 0; i < min_bound.size(); i++) {
        if (parameters[i] < min_bound[i]) return true;
        if (parameters[i] > max_bound[i]) return true;
    }
    return false;
}

void
Recombination::check_bounds(const vector<double> &min_bound, const vector<double> &max_bound) throw (std::string) {
    if (min_bound.size() != max_bound.size()) {
        std::stringstream oss;
        oss << "ERROR [file: " << __FILE__ << ", line: " << __LINE__ << "]: length of min_bound (" << min_bound.size() << ") was not equal to length of max_bound (" << max_bound.size() << ")"; 
        throw oss.str();
    }   

    for (uint32_t i = 0; i < min_bound.size(); i++) {
        if (min_bound >= max_bound) {
            std::stringstream oss;
            oss << "ERROR [file: " << __FILE__ << ", line: " << __LINE__ << "]: min_bound[" << i << "] (" << min_bound[i] << ") was >= max_bound[" << i << "] (" << max_bound[i] << ")"; 
            throw oss.str();
        }
    }
}

void
Recombination::check_step(const vector<double> &step) throw (std::string) {
    for (uint32_t i = 0; i < step.size(); i++) {
        if (step[i] <= 0) {
            std::stringstream oss;
            oss << "ERROR [file: " << __FILE__ << ", line: " << __LINE__ << "]: step or radius[" << i << "] (" << step[i] << ") was <= 0)"; 
            throw oss.str();
        }
    }
}


/**
 *  Functions dealing with parameter generation
 */

void
Recombination::random_within(const vector<double> &min_bound, const vector<double> &max_bound, vector<double> &dest, mt19937 &rng, uniform_real_distribution<double> &distribution) {
    if (dest.size() != min_bound.size()) dest.resize(min_bound.size());

    for (uint32_t i = 0; i < min_bound.size(); i++) {
        dest[i] = min_bound[i] + (distribution(rng) * (max_bound[i] - min_bound[i]));
    }
}

void
Recombination::random_around(const vector<double> &center, const vector<double> &radius, vector<double> &dest, mt19937 &rng, uniform_real_distribution<double> &distribution) {
    if (dest.size() != center.size()) dest.resize(center.size());

    for (uint32_t i = 0; i < center.size(); i++) {
        dest[i] = center[i] - radius[i] + (distribution(rng) * 2.0 * radius[i]);
    }
}

//generate a set of parameters randomly along a direction
void
Recombination::random_along(const vector<double> &center, const vector<double> &direction, double ls_min, double ls_max, vector<double> &dest, mt19937 &rng, uniform_real_distribution<double> &distribution) {
    if (dest.size() != center.size()) dest.resize(center.size());

    double distance = distribution(rng);
    for (uint32_t i = 0; i < center.size(); i++) {
        dest[i] = center[i] + (ls_min * direction[i]) + (distance * (ls_max - ls_min) * direction[i]);
    }
}

void
Recombination::binary_recombination(const vector<double> &src1, const vector<double> &src2, double crossover_rate, vector<double> &dest, mt19937 &rng, uniform_real_distribution<double> &distribution) {
    uint32_t selected = (uint32_t)(distribution(rng) * src1.size());

    if (dest.size() != src1.size()) dest.resize(src1.size());

    for (uint32_t i = 0; i < src1.size(); i++) {
        if (i == selected || distribution(rng) < crossover_rate) {
            dest[i] = src2[i];
        } else {
            dest[i] = src1[i];
        }
    }
}

void
Recombination::exponential_recombination(const vector<double> &src1, const vector<double> &src2, double crossover_rate, vector<double> &dest, mt19937 &rng, uniform_real_distribution<double> &distribution) {
    uint32_t selected = (uint32_t)(distribution(rng) * src1.size());

    if (dest.size() != src1.size()) dest.resize(src1.size());

    uint32_t i;
    for (i = 0; i < src1.size(); i++) {
        if (i == selected || distribution(rng) < crossover_rate) break;
        dest[i] = src1[i];
    }

    for (; i < src1.size(); i++) dest[i] = src2[i];
}
