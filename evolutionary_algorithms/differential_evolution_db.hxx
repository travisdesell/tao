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

#ifndef TAO_DIFFERENTIAL_EVOLUTION_DB
#define TAO_DIFFERENTIAL_EVOLUTION_DB

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "evolutionary_algorithm_db.hxx"
#include "differential_evolution.hxx"

#include "mysql.h"

class DifferentialEvolutionDB : public DifferentialEvolution, public EvolutionaryAlgorithmDB {
    protected:
//        int id;
//        std::string name;

        MYSQL *conn;

        void check_name(std::string name) throw (std::string);
    public:
        DifferentialEvolutionDB(MYSQL *conn, std::string name) throw (std::string);
        DifferentialEvolutionDB(MYSQL *conn, int id) throw (std::string);

        DifferentialEvolutionDB( MYSQL *conn,
                                 const int32_t app_id,
                                 const std::vector<std::string> &arguments) throw (std::string);

        DifferentialEvolutionDB( MYSQL *conn,
                                 const std::vector<std::string> &arguments) throw (std::string);

        DifferentialEvolutionDB( MYSQL *conn,
                                 const int32_t app_id,
                                 const std::vector<double> &min_bound,                                    /* min bound is copied into the search */
                                 const std::vector<double> &max_bound,                                    /* max bound is copied into the search */
                                 const std::vector<std::string> &arguments) throw (std::string);          /* initialize the DE from command line arguments */

        DifferentialEvolutionDB( MYSQL *conn,
                                 const std::vector<double> &min_bound,                                    /* min bound is copied into the search */
                                 const std::vector<double> &max_bound,                                    /* max bound is copied into the search */
                                 const std::vector<std::string> &arguments) throw (std::string);          /* initialize the DE from command line arguments */

        DifferentialEvolutionDB( MYSQL *conn,
                                 const int32_t app_id,
                                 const std::string name,
                                 const std::vector<double> &min_bound,                                    /* min bound is copied into the search */
                                 const std::vector<double> &max_bound,                                    /* max bound is copied into the search */
                                 const uint32_t population_size,
                                 const uint16_t parent_selection,                                         /* How to select the parent */
                                 const uint16_t number_pairs,                                             /* How many individuals to used to calculate differntials */
                                 const uint16_t recombination_selection,                                  /* How to perform recombination */
                                 const double parent_scaling_factor,                                      /* weight for the parent calculation*/
                                 const double differential_scaling_factor,                                /* weight for the differential calculation */
                                 const double crossover_rate,                                             /* crossover rate for recombination */
                                 const bool directional,                                                  /* used for directional calculation of differential (this options is not really a recombination) */
                                 const uint32_t maximum_iterations                                        /* default value is 0 which means no termination */
                               ) throw (std::string);

        DifferentialEvolutionDB( MYSQL *conn,
                                 const std::string name,
                                 const std::vector<double> &min_bound,                                    /* min bound is copied into the search */
                                 const std::vector<double> &max_bound,                                    /* max bound is copied into the search */
                                 const uint32_t population_size,
                                 const uint16_t parent_selection,                                         /* How to select the parent */
                                 const uint16_t number_pairs,                                             /* How many individuals to used to calculate differntials */
                                 const uint16_t recombination_selection,                                  /* How to perform recombination */
                                 const double parent_scaling_factor,                                      /* weight for the parent calculation*/
                                 const double differential_scaling_factor,                                /* weight for the differential calculation */
                                 const double crossover_rate,                                             /* crossover rate for recombination */
                                 const bool directional,                                                  /* used for directional calculation of differential (this options is not really a recombination) */
                                 const uint32_t maximum_iterations                                        /* default value is 0 which means no termination */
                               ) throw (std::string);

        DifferentialEvolutionDB( MYSQL *conn,
                                 const int32_t app_id,
                                 const std::string name,
                                 const std::vector<double> &min_bound,                                    /* min bound is copied into the search */
                                 const std::vector<double> &max_bound,                                    /* max bound is copied into the search */
                                 const uint32_t population_size,
                                 const uint16_t parent_selection,                                         /* How to select the parent */
                                 const uint16_t number_pairs,                                             /* How many individuals to used to calculate differntials */
                                 const uint16_t recombination_selection,                                  /* How to perform recombination */
                                 const double parent_scaling_factor,                                      /* weight for the parent calculation*/
                                 const double differential_scaling_factor,                                /* weight for the differential calculation */
                                 const double crossover_rate,                                             /* crossover rate for recombination */
                                 const bool directional,                                                  /* used for directional calculation of differential (this options is not really a recombination) */
                                 const uint32_t maximum_created,                                          /* default value is 0 which means no termination */
                                 const uint32_t maximum_reported                                          /* default value is 0 which means no termination */
                               ) throw (std::string);


        DifferentialEvolutionDB( MYSQL *conn,
                                 const std::string name,
                                 const std::vector<double> &min_bound,                                    /* min bound is copied into the search */
                                 const std::vector<double> &max_bound,                                    /* max bound is copied into the search */
                                 const uint32_t population_size,
                                 const uint16_t parent_selection,                                         /* How to select the parent */
                                 const uint16_t number_pairs,                                             /* How many individuals to used to calculate differntials */
                                 const uint16_t recombination_selection,                                  /* How to perform recombination */
                                 const double parent_scaling_factor,                                      /* weight for the parent calculation*/
                                 const double differential_scaling_factor,                                /* weight for the differential calculation */
                                 const double crossover_rate,                                             /* crossover rate for recombination */
                                 const bool directional,                                                  /* used for directional calculation of differential (this options is not really a recombination) */
                                 const uint32_t maximum_created,                                          /* default value is 0 which means no termination */
                                 const uint32_t maximum_reported                                          /* default value is 0 which means no termination */
                               ) throw (std::string);

        ~DifferentialEvolutionDB();

        static bool search_exists(MYSQL *conn, std::string search_name) throw (std::string);
        static void create_tables(MYSQL *conn) throw (std::string);

        void construct_from_database(std::string query) throw (std::string);
        void construct_from_database(MYSQL_ROW row) throw (std::string);
        void insert_to_database() throw (std::string);           /* Insert a particle swarm into the database */

        /**
         *  The following methods are used for asynchronous optimization
         */
        virtual void new_individual(uint32_t &id, std::vector<double> &parameters) throw (std::string);
        virtual void new_individual(uint32_t &id, std::vector<double> &parameters, uint32_t &seed) throw (std::string);
        virtual bool insert_individual(uint32_t id, const std::vector<double> &parameters, double fitness, uint32_t seed = 0) throw (std::string);         /* Returns true if the individual was inserted. */

        virtual void update_current_individual() throw (std::string);

        static void add_searches(MYSQL *conn, int32_t app_id, std::vector<EvolutionaryAlgorithmDB*> &searches) throw (std::string);
        static void add_unfinished_searches(MYSQL *conn, int32_t app_id, std::vector<EvolutionaryAlgorithmDB*> &unfinished_searches) throw (std::string);

        void print_to(std::ostream& stream);
        friend std::ostream& operator<< (std::ostream& stream, DifferentialEvolutionDB &ps);
};


#endif
