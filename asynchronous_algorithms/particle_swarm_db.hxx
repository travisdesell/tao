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

#ifndef TAO_PARTICLE_SWARM_DB
#define TAO_PARTICLE_SWARM_DB

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "evolutionary_algorithm_db.hxx"
#include "particle_swarm.hxx"

#include "mysql.h"

class ParticleSwarmDB : public ParticleSwarm, public EvolutionaryAlgorithmDB {
    protected:
//        int id;
//        std::string name;

        MYSQL *conn;

        void check_name(std::string name) throw (std::string);

    public:
        ParticleSwarmDB(MYSQL *conn, std::string name) throw (std::string);
        ParticleSwarmDB(MYSQL *conn, int id) throw (std::string);

        ParticleSwarmDB( MYSQL *conn,
                         const std::vector<std::string> &arguments) throw (std::string);

        ParticleSwarmDB( MYSQL *conn,
                         const int32_t app_id,
                         const std::vector<std::string> &arguments) throw (std::string);

        ParticleSwarmDB( MYSQL *conn,
                         const std::vector<double> &min_bound,              /* min bound is copied into the search */
                         const std::vector<double> &max_bound,              /* max bound is copied into the search */
                         const std::vector<std::string> &arguments
                       ) throw (std::string);

        ParticleSwarmDB( MYSQL *conn,
                         const int32_t app_id,
                         const std::vector<double> &min_bound,              /* min bound is copied into the search */
                         const std::vector<double> &max_bound,              /* max bound is copied into the search */
                         const std::vector<std::string> &arguments
                       ) throw (std::string);

        ParticleSwarmDB( MYSQL *conn,
                         const std::string name,
                         const std::vector<double> &min_bound,              /* min bound is copied into the search */
                         const std::vector<double> &max_bound,              /* max bound is copied into the search */
                         const uint32_t population_size,
                         const double inertia,                              /* intertia */
                         const double global_best_weight,                   /* global best weight */
                         const double local_best_weight,                    /* local best weight */
                         const double initial_velocity_scale,               /* A scale for the initial velocities of particles so it doesn't immediately go to the bounds */
                         const uint32_t maximum_iterations                  /* default value is 0 which means no termination */
                       ) throw (std::string);

        ParticleSwarmDB( MYSQL *conn,
                         const int32_t app_id,
                         const std::string name,
                         const std::vector<double> &min_bound,              /* min bound is copied into the search */
                         const std::vector<double> &max_bound,              /* max bound is copied into the search */
                         const uint32_t population_size,
                         const double inertia,                              /* intertia */
                         const double global_best_weight,                   /* global best weight */
                         const double local_best_weight,                    /* local best weight */
                         const double initial_velocity_scale,               /* A scale for the initial velocities of particles so it doesn't immediately go to the bounds */
                         const uint32_t maximum_iterations                  /* default value is 0 which means no termination */
                       ) throw (std::string);

        ParticleSwarmDB( MYSQL *conn,
                         const std::string name,
                         const std::vector<double> &min_bound,              /* min bound is copied into the search */
                         const std::vector<double> &max_bound,              /* max bound is copied into the search */
                         const uint32_t population_size,
                         const double inertia,                              /* intertia */
                         const double global_best_weight,                   /* global best weight */
                         const double local_best_weight,                    /* local best weight */
                         const double initial_velocity_scale,               /* A scale for the initial velocities of particles so it doesn't immediately go to the bounds */
                         const uint32_t maximum_created,                    /* default value is 0 which means no termination */
                         const uint32_t maximum_reported                    /* default value is 0 which means no termination */
                       ) throw (std::string);

        ParticleSwarmDB( MYSQL *conn,
                         const int32_t app_id,
                         const std::string name,
                         const std::vector<double> &min_bound,              /* min bound is copied into the search */
                         const std::vector<double> &max_bound,              /* max bound is copied into the search */
                         const uint32_t population_size,
                         const double inertia,                              /* intertia */
                         const double global_best_weight,                   /* global best weight */
                         const double local_best_weight,                    /* local best weight */
                         const double initial_velocity_scale,               /* A scale for the initial velocities of particles so it doesn't immediately go to the bounds */
                         const uint32_t maximum_created,                    /* default value is 0 which means no termination */
                         const uint32_t maximum_reported                    /* default value is 0 which means no termination */
                       ) throw (std::string);


        ~ParticleSwarmDB();

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
        friend std::ostream& operator<< (std::ostream& stream, ParticleSwarmDB &ps);
};


#endif
