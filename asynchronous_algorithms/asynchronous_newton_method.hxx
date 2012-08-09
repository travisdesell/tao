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

//From undvc_common
#include "undvc_common/vector_io.hxx"
#include "undvc_common/arguments.hxx"

//From Boost
#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"


using namespace std;

class AsynchronousNewtonMethod {
    private:
        bool min_bound_defined, max_bound_defined;
        vector<double> max_bound;
        vector<double> min_bound;

        bool regression_radius_defined, center_defined;
        vector<double> regression_radius;
        vector<double> center;

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
        uint32_t iteration;

        boost::variate_generator< boost::mt19937, boost::uniform_real<> > *random_number_generator;

    public:
        ~AsynchronousNewtonMethod();

        AsynchronousNewtonMethod();
        AsynchronousNewtonMethod(const vector<string> &arguments) throw (string);

        AsynchronousNewtonMethod(
                                const vector<double> &min_bound,
                                const vector<double> &max_bound,
                                const vector<double> regression_width,
                                const vector<string> &arguments
                            ) throw (string);

        AsynchronousNewtonMethod(
                                const vector<double> &min_bound,
                                const vector<double> &max_bound,
                                const vector<double> current_center,
                                const vector<double> regression_width,
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

        void parse_arguments(const vector<string> &arguments);
        void pre_initialize();
        void initialize();

        //returns true if it generates individuals
        bool generate_individuals(uint32_t &number_individuals, uint32_t &iteration, vector< vector<double> > &parameters, vector<uint32_t> &seeds) throw (string);
        bool generate_individuals(uint32_t &number_individuals, uint32_t &iteration, vector< vector<double> > &parameters) throw (string);

        //returns true if modified
        bool insert_individual(uint32_t iteration, const vector<double> &parameters, double fitness, uint32_t seed) throw (string);
        bool insert_individual(uint32_t iteration, const vector<double> &parameters, double fitness) throw (string);

        void iterate(double (*objective_function)(const vector<double> &)) throw (string);
        void iterate(double (*objective_function)(const vector<double> &, const uint32_t)) throw (string);
};
