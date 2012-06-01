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
        static void add_unfinished_searches(MYSQL *conn, int32_t app_id, std::vector<EvolutionaryAlgorithmDB*> &unfinished_searches) throw (std::string);

        void print_to(std::ostream& stream);
        friend std::ostream& operator<< (std::ostream& stream, ParticleSwarmDB &ps);
};


#endif
