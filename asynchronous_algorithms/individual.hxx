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

#ifndef TAO_EA_INDIVIDUAL_H
#define TAO_EA_INDIVIDUAL_H

#include <iostream>
#include <vector>

#include "util/vector_io.hxx"

using std::cerr;
using std::endl;

using std::string;
using std::vector;

class Individual {
    public:
        int position;
        double fitness;
        vector<double> parameters;
        string metadata;

        Individual(int position, double fitness, const vector<double> &parameters, const string &metadata) : position(position), fitness(fitness), parameters(parameters), metadata(metadata) {}
        virtual ~Individual() {}

        bool operator<(const Individual &rhs);
        friend bool operator<(const Individual &lhs, const Individual &rhs);

        Individual& operator=(const Individual& rhs);

        friend std::ostream& operator<< (std::ostream& stream, const Individual &individual);
};

#endif
