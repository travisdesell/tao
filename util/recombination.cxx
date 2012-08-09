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

#include "recombination.hxx"

#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"

using boost::variate_generator;
using boost::mt19937;
using boost::uniform_real;

using std::vector;


/**
 *  Functions dealing with bounds.
 */
void
Recombination::bound_parameters(const vector<double> &min_bound, const vector<double> &max_bound, vector<double> &dest) {
    for (uint32_t i = 0; i < min_bound.size(); i++) {
        if (dest[i] < min_bound[i]) dest[i] = min_bound[i];
        if (dest[i] > max_bound[i]) dest[i] = max_bound[i];
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
            oss << "ERROR [file: " << __FILE__ << ", line: " << __LINE__ << "]: step or width[" << i << "] (" << step[i] << ") was <= 0)"; 
            throw oss.str();
        }
    }
}


/**
 *  Functions dealing with parameter generation
 */

void
Recombination::random_within(const vector<double> &min_bound, const vector<double> &max_bound, vector<double> &dest, variate_generator< mt19937,uniform_real<> > *rng) {
    if (dest.size() != min_bound.size()) dest.resize(min_bound.size());

    for (uint32_t i = 0; i < min_bound.size(); i++) {
        dest[i] = min_bound[i] + ((*rng)() * (max_bound[i] - min_bound[i]));
    }
}

void
Recombination::random_around(const vector<double> &center, const vector<double> &width, vector<double> &dest, variate_generator< mt19937,uniform_real<> > *rng) {
    if (dest.size() != center.size()) dest.resize(center.size());

    for (uint32_t i = 0; i < center.size(); i++) {
        dest[i] = center[i] - width[i] + ((*rng)() * 2.0 * width[i]);
    }
}

//generate a set of parameters randomly along a direction
void
Recombination::random_along(const vector<double> &center, const vector<double> &direction, double ls_min, double ls_max, vector<double> &dest, variate_generator< mt19937,uniform_real<> > *rng) {
    if (dest.size() != center.size()) dest.resize(center.size());

    for (uint32_t i = 0; i < center.size(); i++) {
        dest[i] = center[i] + (center[i] - (ls_min * direction[i])) + ((*rng)() * (ls_max - ls_min) * direction[i]);
    }
}

void
Recombination::binary_recombination(const vector<double> &src1, const vector<double> &src2, double crossover_rate, vector<double> &dest, variate_generator< mt19937,uniform_real<> > *rng) {
    uint32_t selected = (uint32_t)((*rng)() * src1.size());

    if (dest.size() != src1.size()) dest.resize(src1.size());

    for (uint32_t i = 0; i < src1.size(); i++) {
        if (i == selected || (*rng)() < crossover_rate) {
            dest[i] = src2[i];
        } else {
            dest[i] = src1[i];
        }
    }
}

void
Recombination::exponential_recombination(const vector<double> &src1, const vector<double> &src2, double crossover_rate, vector<double> &dest, variate_generator< mt19937,uniform_real<> > *rng) {
    uint32_t selected = (uint32_t)((*rng)() * src1.size());

    if (dest.size() != src1.size()) dest.resize(src1.size());

    uint32_t i;
    for (i = 0; i < src1.size(); i++) {
        if (i == selected || (*rng)() < crossover_rate) break;
        dest[i] = src1[i];
    }

    for (; i < src1.size(); i++) dest[i] = src2[i];
}
