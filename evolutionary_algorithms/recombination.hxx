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
#include <stdint.h>
#include <string>

#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"

using boost::variate_generator;
using boost::mt19937;
using boost::uniform_real;


using std::vector;

class Recombination {
    public:

        static void bound_parameters(const vector<double> &min_bound, const vector<double> &max_bound, vector<double> &dest);

        static void check_bounds(const vector<double> &min_bound, const vector<double> &max_bound) throw (std::string);

        static bool out_of_bounds(const vector<double> &min_bound, const vector<double> &max_bound, const vector<double> &parameters);

        static void random_parameters(const vector<double> &min_bound, const vector<double> &max_bound, vector<double> &dest, variate_generator< mt19937,uniform_real<> > *rng);

        static void binary_recombination(const vector<double> &src1, const vector<double> &src2, double crossover_rate, vector<double> &dest, variate_generator< mt19937,uniform_real<> > *rng);

        static void exponential_recombination(const vector<double> &src1, const vector<double> &src2, double crossover_rate, vector<double> &dest, variate_generator< mt19937,uniform_real<> > *rng);
};

#endif
