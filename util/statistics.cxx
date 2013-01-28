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
#include <algorithm>
#include <numeric>
#include <limits>

#include "stdint.h"

#include "asynchronous_algorithms/individual.hxx"
#include "util/statistics.hxx"

using namespace std;

void calculate_fitness_statistics(const vector<Individual> &individuals, double &best, double &average, double &median, double &worst) {
    vector<Individual> individuals_copy(individuals);

    sort(individuals_copy.begin(), individuals_copy.end());

    best = individuals_copy[individuals_copy.size() - 1].fitness;
    median = individuals_copy[individuals_copy.size() / 2].fitness;
    worst = individuals_copy[0].fitness;

    average = 0;
    for (uint32_t i = 0; i < individuals_copy.size(); i++) {
        average += individuals_copy[i].fitness;
    }
    average = average / individuals_copy.size();
}

void calculate_fitness_statistics(const vector<double> &fitness, double &best, double &average, double &median, double &worst) {
    vector<double> fitness_copy(fitness);

    sort(fitness_copy.begin(), fitness_copy.end());

    best = fitness_copy[fitness_copy.size() - 1];
    median = fitness_copy[fitness_copy.size() / 2];
    worst = fitness_copy[0];

    average = 0;
    for (uint32_t i = 0; i < fitness_copy.size(); i++) {
        average += fitness_copy[i];
    }
    average = average / fitness_copy.size();

    if (average == -numeric_limits<double>::infinity()) {
        average = -numeric_limits<double>::max();
    }
}
