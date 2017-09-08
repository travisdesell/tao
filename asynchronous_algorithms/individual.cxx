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

#include <cstdlib>
#include <iostream>
#include <vector>

#include "individual.hxx"
#include "util/vector_io.hxx"

using std::cerr;
using std::endl;

using std::string;
using std::vector;


bool
Individual::operator<(const Individual &rhs) {
    return fitness < rhs.fitness;
}

bool operator<(const Individual &lhs, const Individual &rhs) {
    return lhs.fitness < rhs.fitness;
}

Individual& 
Individual::operator=(const Individual& rhs) { 
    position = rhs.position;
    fitness = rhs.fitness;
    parameters.assign(rhs.parameters.begin(), rhs.parameters.end());
    metadata = rhs.metadata;

    return *this;
}

std::ostream& operator<< (std::ostream& stream, const Individual &individual) {
    stream << individual.position << ", " << individual.fitness << ", " << vector_to_string(individual.parameters) << ", '" << individual.metadata << "'";
    return stream;
}
