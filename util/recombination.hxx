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

#ifndef TAO_RECOMBINATION_H
#define TAO_RECOMBINATION_H

#include <vector>
using std::vector;

#include <stdint.h>
#include <string>

#include <random>
using std::mt19937;
using std::uniform_real_distribution;



class Recombination {
    public:

        static void bound_parameters(const vector<double> &min_bound, const vector<double> &max_bound, vector<double> &dest, bool wrap_radians = false);

        static void check_bounds(const vector<double> &min_bound, const vector<double> &max_bound) throw (std::string);
        static void check_step(const vector<double> &step) throw (std::string);

        static bool out_of_bounds(const vector<double> &min_bound, const vector<double> &max_bound, const vector<double> &parameters);

        //generates a random point within the given bounds
        static void random_within(const vector<double> &min_bound, const vector<double> &max_bound, vector<double> &dest, mt19937 &rng, uniform_real_distribution<double> &distribution);
        //generates a random point around center, with maximum radius
        static void random_around(const vector<double> &center, const vector<double> &radius, vector<double> &dest, mt19937 &rng, uniform_real_distribution<double> &distribution);
        //generates a random point along a line staring at center, specified by direction, and from center + ls_min * direction to center + ls_max * direction
        static void random_along(const vector<double> &center, const vector<double> &direction, double ls_min, double ls_max, vector<double> &dest, mt19937 &rng, uniform_real_distribution<double> &distribution);

        static void binary_recombination(const vector<double> &src1, const vector<double> &src2, double crossover_rate, vector<double> &dest, mt19937 &rng, uniform_real_distribution<double> &distribution);

        static void exponential_recombination(const vector<double> &src1, const vector<double> &src2, double crossover_rate, vector<double> &dest, mt19937 &rng, uniform_real_distribution<double> &distribution);
};

#endif
