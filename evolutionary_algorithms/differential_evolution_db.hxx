#ifndef TAO_DIFFERENTIAL_EVOLUTION_DB
#define TAO_DIFFERENTIAL_EVOLUTION_DB

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "differential_evolution.hxx"

#include "mysql.h"

class DifferentialEvolutionDB : public DifferentialEvolution {
    protected:
        int id;
        std::string name;

        MYSQL *conn;

    public:
        DifferentialEvolutionDB(MYSQL *conn, std::string name) throw (std::string);
        DifferentialEvolutionDB(MYSQL *conn, int id) throw (std::string);

        DifferentialEvolutionDB( MYSQL *conn,
                                 const std::vector<std::string> &arguments) throw (std::string);

        DifferentialEvolutionDB( MYSQL *conn,
                                 const std::vector<double> &min_bound,                                    /* min bound is copied into the search */
                                 const std::vector<double> &max_bound,                                    /* max bound is copied into the search */
                                 const std::vector<std::string> &arguments) throw (std::string);          /* initialize the DE from command line arguments */

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
        virtual bool insert_individual(uint32_t id, const std::vector<double> &parameters, double fitness, uint32_t seed = 0) throw (std::string);         /* Returns true if the individual was inserted. */

        void print_to(std::ostream& stream);
        friend std::ostream& operator<< (std::ostream& stream, DifferentialEvolutionDB &ps);
};


#endif
