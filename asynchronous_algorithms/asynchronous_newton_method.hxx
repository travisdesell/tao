#ifndef TAO_ASYNCHRONOUS_NEWTON_METHOD_H
#define TAO_ASYNCHRONOUS_NEWTON_METHOD_H

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

#include <string>
#include <vector>
#include <limits>
#include <iostream>
#include <iomanip>

#include "util/recombination.hxx"
#include "util/statistics.hxx"


using namespace std;

class AsynchronousNewtonMethod {
    protected:
        bool min_bound_defined, max_bound_defined;
        vector<double> max_bound;
        vector<double> min_bound;

        bool regression_radius_defined, center_defined;
        vector<double> regression_radius;
        vector<double> center;

        double center_fitness;
        uint32_t failed_improvements;

        bool minimum_regression_individuals_defined;
        uint32_t minimum_regression_individuals;

        bool minimum_line_search_individuals_defined;
        uint32_t minimum_line_search_individuals;

        bool extra_workunits_defined;
        uint32_t extra_workunits;

        vector< vector<double> > regression_individuals;
        vector<double> regression_fitnesses;
        vector<uint32_t> regression_seeds;

        vector<double> line_search_direction;

        bool line_search_min_defined, line_search_max_defined;
        double line_search_min;
        double line_search_max;

        vector< vector<double> > line_search_individuals;
        vector<double> line_search_fitnesses;
        vector<uint32_t> line_search_seeds;

        uint64_t line_search_individuals_reported;
        uint64_t regression_individuals_reported;

        uint32_t number_parameters;

        bool maximum_iterations_defined;
        uint32_t maximum_iterations;

        bool first_workunits_generated;
        uint32_t current_iteration;

        bool max_failed_improvements_defined;
        uint32_t max_failed_improvements;

        mt19937 random_number_generator;
        uniform_real_distribution<double> random_0_1;

        AsynchronousNewtonMethod();
    public:
        ~AsynchronousNewtonMethod();

        AsynchronousNewtonMethod(const vector<string> &arguments) throw (string);

        AsynchronousNewtonMethod(
                                const vector<double> &min_bound,
                                const vector<double> &max_bound,
                                const vector<double> &regression_width,
                                const vector<string> &arguments
                            ) throw (string);

        AsynchronousNewtonMethod(
                                const vector<double> &min_bound,
                                const vector<double> &max_bound,
                                const vector<double> &current_center,
                                const vector<double> &regression_width,
                                const vector<string> &arguments
                            ) throw (string);

        AsynchronousNewtonMethod(
                                const vector<double> &min_bound,                  /* min bound is copied into the search */
                                const vector<double> &max_bound,                  /* max bound is copied into the search */
                                const vector<double> &current_center,
                                const vector<double> &regression_width,
                                const uint32_t minimum_regression_individuals,
                                const uint32_t minimum_line_search_individuals,
                                const uint32_t maximum_iterations                 /* default value is 0 which means no termination */
                            ) throw (string);

        void initialize_rng();
        void parse_arguments(const vector<string> &arguments);
        void pre_initialize();
        void initialize();

        //returns true if it generates individuals
        virtual bool generate_individuals(uint32_t &number_individuals, uint32_t &iteration, vector< vector<double> > &parameters, vector<uint32_t> &seeds) throw (string);
        virtual bool generate_individuals(uint32_t &number_individuals, uint32_t &iteration, vector< vector<double> > &parameters) throw (string);

        //returns true if modified
        virtual bool insert_individual(uint32_t iteration, const vector<double> &parameters, double fitness, uint32_t seed) throw (string);
        virtual bool insert_individual(uint32_t iteration, const vector<double> &parameters, double fitness) throw (string);

        void iterate(double (*objective_function)(const vector<double> &)) throw (string);
        void iterate(double (*objective_function)(const vector<double> &, const uint32_t)) throw (string);
};

#endif
