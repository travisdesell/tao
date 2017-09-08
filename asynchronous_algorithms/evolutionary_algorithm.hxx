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

#ifndef TAO_EVOLUTIONARY_ALGORITHM_H
#define TAO_EVOLUTIONARY_ALGORITHM_H

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include <random>
using std::mt19937;
using std::uniform_real_distribution;

#include <stdint.h>

#include "individual.hxx"

class EvolutionaryAlgorithm {
    protected:
        //For iterative EAs
        uint32_t maximum_iterations;
        uint32_t current_iteration;

        //For asynchronous EAs
        uint32_t maximum_created;
        uint32_t individuals_created;
        uint32_t maximum_reported;
        uint32_t individuals_reported;

        bool wrap_radians;

        uint32_t number_parameters;
        std::vector<double> min_bound;
        std::vector<double> max_bound;

        uint32_t current_individual;

        uint32_t population_size;
        std::vector<uint32_t> seeds;

        bool quiet;
        double start_time;

        //For random number generation
        mt19937 random_number_generator;
        uniform_real_distribution<double> random_0_1;

        std::ofstream *log_file;

        EvolutionaryAlgorithm();

        void initialize();
        void initialize_rng();
        void parse_arguments(const std::vector<std::string> &arguments);


    public:
        uint32_t get_population_size()      { return population_size; }
        uint32_t get_current_individual()   { return current_individual; }
        uint32_t get_current_iteration()    { return current_iteration; }
        uint32_t get_individuals_created()  { return individuals_created; }
        uint32_t get_number_parameters()    { return number_parameters; }

        bool is_running() {
            return (maximum_reported == 0 || individuals_reported < maximum_reported) &&
                   (maximum_created ==  0 || individuals_created  < maximum_created) &&
                   (maximum_iterations == 0 || current_iteration < maximum_iterations);

        }

        void set_log_file(std::ofstream *log_file);

        /**
         *  Create/delete an EvolutionaryAlgorithm
         */
        EvolutionaryAlgorithm( const std::vector<std::string> &arguments    /* initialize the DE from command line arguments */
                             ) throw (std::string);

        EvolutionaryAlgorithm( const std::vector<double> &min_bound,        /* min bound is copied into the search */
                               const std::vector<double> &max_bound,        /* max bound is copied into the search */
                               const std::vector<std::string> &arguments    /* initialize the DE from command line arguments */
                             ) throw (std::string);

        EvolutionaryAlgorithm( const std::vector<double> &min_bound,    /* min bound is copied into the search */
                               const std::vector<double> &max_bound,    /* max bound is copied into the search */
                               const uint32_t population_size,
                               const uint32_t maximum_iterations        /* default value is 0 which means no termination */
                             ) throw (std::string);

        EvolutionaryAlgorithm( const std::vector<double> &min_bound,    /* min bound is copied into the search */
                               const std::vector<double> &max_bound,    /* max bound is copied into the search */
                               const uint32_t population_size,
                               const uint32_t maximum_created,          /* default value is 0 which means no termination */
                               const uint32_t maximum_reported          /* default value is 0 which means no termination */
                             ) throw (std::string);



        virtual ~EvolutionaryAlgorithm();

        /**
         *  The following methods are used for asynchronous optimization and are purely virtual
         */
        virtual void new_individual(uint32_t &id, std::vector<double> &parameters) throw (std::string) = 0;
        virtual void new_individual(uint32_t &id, std::vector<double> &parameters, uint32_t &seed) throw (std::string) = 0;
        virtual bool insert_individual(uint32_t id, const std::vector<double> &parameters, double fitness, uint32_t seed = 0) throw (std::string) = 0;     /* Returns true if the individual is inserted. */
        virtual bool would_insert(uint32_t id, double fitness) = 0;                                                                     /* Returns true if the individual would be inserted. */

        /**
         *  The following method is for synchronous optimization and is purely virtual
         */
        virtual void iterate(double (*objective_function)(const std::vector<double> &)) throw (std::string) = 0;
        virtual void iterate(double (*objective_function)(const std::vector<double> &, const uint32_t seed)) throw (std::string) = 0;

        virtual void get_individuals(std::vector<Individual> &individuals) = 0;
};

#endif
