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

#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <limits>
#include <iomanip>

#include "evolutionary_algorithm_db.hxx"
#include "differential_evolution_db.hxx"

#include "util/arguments.hxx"
#include "util/statistics.hxx"
#include "util/vector_io.hxx"

/**
 *  From MYSQL
 */
#include "mysql.h"

using namespace std;

void
DifferentialEvolutionDB::check_name(string name) throw (string) {
    if (name.substr(0,3).compare("de_") != 0) {
        ostringstream err_msg;
        err_msg << "Improper name for DifferentialEvolutionDB '" << name << "' must start with 'de_'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw err_msg.str();
    }
}

/**
 *  The following construct a DifferentialEvolution from a database entry
 */
DifferentialEvolutionDB::DifferentialEvolutionDB(MYSQL *conn, string name) throw (string) {
    check_name(name);
    this->conn = conn;

    ostringstream oss;
    oss << "SELECT * FROM differential_evolution WHERE name = '" << name << "'";
//    cout << oss.str() << endl;

    construct_from_database(oss.str());
}

DifferentialEvolutionDB::DifferentialEvolutionDB(MYSQL *conn, int id) throw (string) {
    this->conn = conn;

    ostringstream oss;
    oss << "SELECT * FROM differential_evolution WHERE id = " << id;
//    cout << oss.str() << endl;

    construct_from_database(oss.str());
}

bool
DifferentialEvolutionDB::search_exists(MYSQL *conn, string search_name) throw (string) {
    ostringstream query;
    query << "SELECT id FROM differential_evolution where name = '" << search_name << "'";

    if (mysql_query(conn, query.str().c_str())) {
        ostringstream ex_msg;
        ex_msg << "ERROR: could query database: '" << query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (result != NULL) {
        if (mysql_num_rows(result) > 0) return true;
    } else {
        ostringstream ex_msg;
        ex_msg << "ERROR: could not get result from database from query: '" << query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }

    return false;
}

void
DifferentialEvolutionDB::create_tables(MYSQL *conn) throw (string) {
    ostringstream de_query;
    de_query << "CREATE TABLE `differential_evolution` ("
                << "    `id` int(11) NOT NULL AUTO_INCREMENT,"
                << "    `name` varchar(254) NOT NULL DEFAULT '',"
                << "    `parent_selection` int(11) NOT NULL DEFAULT '0', "
                << "    `number_pairs` int(11) NOT NULL DEFAULT '0', "
                << "    `recombination_selection` int(11) NOT NULL DEFAULT '0', "
                << "    `parent_scaling_factor` double NOT NULL default  '1', "
                << "    `differential_scaling_factor` double NOT NULL default  '1', "
                << "    `crossover_rate` double NOT NULL default  '0.5', "
                << "    `directional` tinyint(1) NOT NULL default  '0', "
                << "    `current_individual` int(11) NOT NULL DEFAULT '0',"
                << "    `initialized_individuals` int(11) NOT NULL DEFAULT '0',"
                << "    `current_iteration` int(11) NOT NULL DEFAULT '0',"
                << "    `maximum_iterations` int(11) NOT NULL DEFAULT '0',"
                << "    `individuals_created` int(11) NOT NULL DEFAULT '0',"
                << "    `maximum_created` int(11) NOT NULL DEFAULT '0',"
                << "    `individuals_reported` int(11) NOT NULL DEFAULT '0',"
                << "    `maximum_reported` int(11) NOT NULL DEFAULT '0',"
                << "    `population_size` int(11) NOT NULL DEFAULT '0',"
                << "    `min_bound` varchar(2048) NOT NULL,"
                << "    `max_bound` varchar(2048) NOT NULL,"
                << "    `app_id`    int(11) NOT NULL DEFAULT '-1',"
                << "    `wrap_radians` tinyint(1) NOT NULL default '0',"
                << "PRIMARY KEY (`id`),"
                << "UNIQUE KEY `name` (`name`)"
                << ") ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=latin1";

    cout << "creating differential_evolution table with: " << endl << de_query.str() << endl << endl;

    if (mysql_query(conn, de_query.str().c_str())) {
        ostringstream ex_msg;
        ex_msg << "ERROR: could not create differential evolution table with query: '" << de_query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }

    ostringstream individual_query;
    individual_query  << "CREATE TABLE `de_individual` ("
                      << "    `differential_evolution_id` int(11) NOT NULL,"
                      << "    `position` int(11) NOT NULL,"
                      << "    `fitness` double NOT NULL,"
                      << "    `parameters` varchar(2048) NOT NULL,"
                      << "    `seed` int(32) UNSIGNED,"
                      << "PRIMARY KEY (`differential_evolution_id`,`position`)"
                      << ") ENGINE=InnoDB DEFAULT CHARSET=latin1";

    cout << "creating de_individual table with: " << endl << individual_query.str() << endl << endl;

    /**
     *  Create the log
     */
    //create table `particle_swarm_log` (`swarm_id` int(11) not null, `evaluation` int(11) not null, `fitness` double, `particle` int(11) not null, `r1` double, `r2` double, `global` bool not null, PRIMARY KEY(`swarm_id`, `evaluation`)) ENGINE=InnoDB;

    if (mysql_query(conn, individual_query.str().c_str())) {
        ostringstream ex_msg;
        ex_msg << "ERROR: could not create de_individual table with query: '" << individual_query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }
}

void 
DifferentialEvolutionDB::construct_from_database(string query) throw (string) {
    mysql_query(conn, query.c_str());

    if (mysql_errno(conn) != 0) {
        ostringstream ex_msg;
        ex_msg << "ERROR: could not get differential evolution from query: '" << query << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }

    MYSQL_RES *result = mysql_store_result(conn);

    if (result != NULL) {
        MYSQL_ROW row = mysql_fetch_row(result);
        
        if (row == NULL) {
            ostringstream ex_msg;
            ex_msg << "ERROR: could not construct differential evolution '" << name << "' from database, it does not exist. Thrown on " << __FILE__ << ":" << __LINE__;
            throw ex_msg.str();
        }

        construct_from_database(row);
        mysql_free_result(result);
    } else {
        ostringstream ex_msg;
        ex_msg << "ERROR: could not get differential evolution from query: '" << query << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }
}


void 
DifferentialEvolutionDB::construct_from_database(MYSQL_ROW row) throw (string) {
    id = atoi(row[0]);
    name = row[1];

    //Inherited from DifferentialEvolution
    parent_selection = atoi(row[2]);
    number_pairs = atoi(row[3]);
    recombination_selection = atoi(row[4]);
    parent_scaling_factor = atof(row[5]);
    differential_scaling_factor = atof(row[6]);
    crossover_rate = atof(row[7]);
    directional = atoi(row[8]);

    current_individual = atoi(row[9]);
    initialized_individuals = atoi(row[10]);

    //inherited from EvolutionaryAlgorithm
    current_iteration = atoi(row[11]);
    maximum_iterations = atoi(row[12]);
    individuals_created = atoi(row[13]);
    maximum_created = atoi(row[14]);
    individuals_reported = atoi(row[15]);
    maximum_reported = atoi(row[16]);

    population_size = atoi(row[17]);
    string_to_vector<double>(row[18], min_bound);
    string_to_vector<double>(row[19], max_bound);
    app_id = atoi(row[20]);
    wrap_radians = atoi(row[21]);
    number_parameters = min_bound.size();

    //Get the individual information from the database
    ostringstream oss;
    oss << "SELECT position, fitness, parameters, seed FROM de_individual WHERE differential_evolution_id = " << this->id << " ORDER BY position";
    mysql_query(conn, oss.str().c_str());
    MYSQL_RES *result = mysql_store_result(conn);

//    cout << oss.str() << endl;

    fitnesses.resize(population_size, -numeric_limits<double>::max());
    population.resize(population_size, vector<double>(number_parameters, 0.0));
    seeds.resize(population_size, 0);

    EvolutionaryAlgorithm::initialize_rng();    //to initialize the random number generator

    if (result != NULL) {
        uint32_t num_results = mysql_num_rows(result);
        if (num_results != population_size) {
            ostringstream ex_msg;
            ex_msg << "ERROR: got " << num_results << " results when looking up individuals for search " << name << ", with a population size: " << population_size << ". Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
            throw ex_msg.str();
        }   

        MYSQL_ROW individual_row;

        while ((individual_row = mysql_fetch_row(result))) {
            int individual_id = atoi(individual_row[0]);
            fitnesses[individual_id] = atof(individual_row[1]);

            if (fitnesses[individual_id] < -1.79768e+308) {
                fitnesses[individual_id] = -numeric_limits<double>::max();
            }

            string_to_vector<double>(individual_row[2], population[individual_id]);
            seeds[individual_id] = atoi(individual_row[3]);

//            cout   << "    [DEIndividual" << endl
//                   << "        position = " << individual_id << endl
//                   << "        fitness = " << fitnesses[individual_id] << endl
//                   << "        parameters = '" << vector_to_string<double>(population[individual_id]) << "'" << endl
//                   << "    ]" << endl;

         }   
        mysql_free_result(result);
    } else {
        ostringstream ex_msg;
        ex_msg << "ERROR: looking up individuals with query: '" << oss.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }

    //calculate global_best and global_best_fitness
    global_best_fitness = -numeric_limits<double>::max();
    for (uint32_t i = 0; i < population.size(); i++) {
        if (global_best_fitness < fitnesses[i]) {
            global_best_id = i;
            global_best_fitness = fitnesses[i];
        }
    }
}


/**
 *  Insert a created differential evolution to a database.
 */
void
DifferentialEvolutionDB::insert_to_database() throw (string) {
    ostringstream query;

    query.precision(10);
    query << "INSERT INTO differential_evolution"
          << " SET "
          << "  name = '" << name << "'"
          << ", parent_selection = " << parent_selection
          << ", number_pairs = " << number_pairs
          << ", recombination_selection = " << recombination_selection
          << ", parent_scaling_factor = " << parent_scaling_factor
          << ", differential_scaling_factor = " << differential_scaling_factor
          << ", crossover_rate = " << crossover_rate
          << ", directional = " << directional
          << ", current_individual = " << current_individual
          << ", initialized_individuals = " << initialized_individuals
          << ", current_iteration = " << current_iteration  
          << ", maximum_iterations = " << maximum_iterations
          << ", individuals_created = " << individuals_created
          << ", maximum_created = " << maximum_created  
          << ", individuals_reported = " << individuals_reported
          << ", maximum_reported = " << maximum_reported 
          << ", population_size = " << population_size
          << ", min_bound = '" << vector_to_string<double>(min_bound) << "'"
          << ", max_bound = '" << vector_to_string<double>(max_bound) << "'"
          << ", app_id = " << app_id 
          << ", wrap_radians = " << wrap_radians;

    mysql_query(conn, query.str().c_str());

    MYSQL_RES *result;
    if ((result = mysql_store_result(conn)) == 0 && mysql_field_count(conn) == 0 && mysql_insert_id(conn) != 0) {
        id = mysql_insert_id(conn);
    } else {
        ostringstream ex_msg;
        ex_msg << "ERROR: could not get differential evolution id from insert query: '" << query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }
    mysql_free_result(result);

    for (uint32_t i = 0; i < population_size; i++) {
        ostringstream individual_query;
        individual_query.precision(10);
        individual_query << "INSERT INTO de_individual"
                         << " SET "
                         << "  differential_evolution_id = " << id
                         << ", position = " << i
                         << ", fitness = '" << setprecision(10) << fitnesses[i] << "'"
                         << ", parameters = '" << vector_to_string<double>(population[i]) << "'"
                         << ", seed = " << seeds[i];

        mysql_query(conn, individual_query.str().c_str());

        if (mysql_errno(conn) != 0) {
            ostringstream ex_msg;
            ex_msg << "ERROR: creating de_individual with insert query: '" << individual_query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
            throw ex_msg.str();
        }
        mysql_free_result(result);
    }
}

/**
 *  The following constructors create new DifferentialEvolutions and insert them into the database.
 */
//Create a differential evolution entirely from arguments
DifferentialEvolutionDB::DifferentialEvolutionDB(MYSQL *conn, const vector<string> &arguments) throw (string) : DifferentialEvolution(arguments) {
    this->conn = conn;
    this->app_id = -1;
    get_argument(arguments, "--search_name", true, name);
    check_name(name);
    insert_to_database();
}

//Create a differential evolution entirely from arguments
DifferentialEvolutionDB::DifferentialEvolutionDB(MYSQL *conn, const int32_t app_id, const vector<string> &arguments) throw (string) : DifferentialEvolution(arguments) {
    this->conn = conn;
    this->app_id = app_id;
    get_argument(arguments, "--search_name", true, name);
    check_name(name);
    insert_to_database();
}


//Create a differential evolution from arguments and a given min and max bound
DifferentialEvolutionDB::DifferentialEvolutionDB( MYSQL *conn,
                                  const vector<double> &min_bound,            /* min bound is copied into the search */
                                  const vector<double> &max_bound,            /* max bound is copied into the search */
                                  const vector<string> &arguments
                                ) throw (string) : DifferentialEvolution(min_bound, max_bound, arguments) {
    this->conn = conn;
    this->app_id = -1;
    get_argument(arguments, "--search_name", true, name);
    check_name(name);
    insert_to_database();
}


//Create a differential evolution from arguments and a given min and max bound
DifferentialEvolutionDB::DifferentialEvolutionDB( MYSQL *conn,
                                  const int32_t app_id,
                                  const vector<double> &min_bound,            /* min bound is copied into the search */
                                  const vector<double> &max_bound,            /* max bound is copied into the search */
                                  const vector<string> &arguments
                                ) throw (string) : DifferentialEvolution(min_bound, max_bound, arguments) {
    this->conn = conn;
    this->app_id = app_id;
    get_argument(arguments, "--search_name", true, name);
    check_name(name);
    insert_to_database();
}

//Create a differential evolution entirely from defined parameters.
DifferentialEvolutionDB::DifferentialEvolutionDB( MYSQL *conn,
                                                  const string name,
                                                  const vector<double> &min_bound,                                    /* min bound is copied into the search */
                                                  const vector<double> &max_bound,                                    /* max bound is copied into the search */
                                                  const uint32_t population_size,
                                                  const uint16_t parent_selection,                                         /* How to select the parent */
                                                  const uint16_t number_pairs,                                             /* How many individuals to used to calculate differntials */
                                                  const uint16_t recombination_selection,                                  /* How to perform recombination */
                                                  const double parent_scaling_factor,                                      /* weight for the parent calculation*/
                                                  const double differential_scaling_factor,                                /* weight for the differential calculation */
                                                  const double crossover_rate,                                             /* crossover rate for recombination */
                                                  const bool directional,                                                  /* used for directional calculation of differential (this options is not really a recombination) */
                                                  const uint32_t maximum_iterations                                        /* default value is 0 which means no termination */
                                                ) throw (string) : DifferentialEvolution(min_bound, max_bound, population_size, parent_selection, number_pairs, recombination_selection, parent_scaling_factor, differential_scaling_factor, crossover_rate, directional, maximum_iterations) {
    this->conn = conn;
    this->app_id = -1;
    this->name = name;
    check_name(name);
    insert_to_database();
}


//Create a differential evolution entirely from defined parameters.
DifferentialEvolutionDB::DifferentialEvolutionDB( MYSQL *conn,
                                                  const int32_t app_id,
                                                  const string name,
                                                  const vector<double> &min_bound,                                    /* min bound is copied into the search */
                                                  const vector<double> &max_bound,                                    /* max bound is copied into the search */
                                                  const uint32_t population_size,
                                                  const uint16_t parent_selection,                                         /* How to select the parent */
                                                  const uint16_t number_pairs,                                             /* How many individuals to used to calculate differntials */
                                                  const uint16_t recombination_selection,                                  /* How to perform recombination */
                                                  const double parent_scaling_factor,                                      /* weight for the parent calculation*/
                                                  const double differential_scaling_factor,                                /* weight for the differential calculation */
                                                  const double crossover_rate,                                             /* crossover rate for recombination */
                                                  const bool directional,                                                  /* used for directional calculation of differential (this options is not really a recombination) */
                                                  const uint32_t maximum_iterations                                        /* default value is 0 which means no termination */
                                                ) throw (string) : DifferentialEvolution(min_bound, max_bound, population_size, parent_selection, number_pairs, recombination_selection, parent_scaling_factor, differential_scaling_factor, crossover_rate, directional, maximum_iterations) {
    this->conn = conn;
    this->app_id = app_id;
    this->name = name;
    check_name(name);
    insert_to_database();
}

DifferentialEvolutionDB::DifferentialEvolutionDB( MYSQL *conn,
                                                 const string name,
                                                 const vector<double> &min_bound,                                    /* min bound is copied into the search */
                                                 const vector<double> &max_bound,                                    /* max bound is copied into the search */
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
                                               ) throw (string) : DifferentialEvolution(min_bound, max_bound, population_size, parent_selection, number_pairs, recombination_selection, parent_scaling_factor, differential_scaling_factor, crossover_rate, directional, maximum_created, maximum_reported) {
    this->conn = conn;
    this->app_id = -1;
    this->name = name;
    check_name(name);
    insert_to_database();
}

DifferentialEvolutionDB::DifferentialEvolutionDB( MYSQL *conn,
                                                 const int32_t app_id,
                                                 const string name,
                                                 const vector<double> &min_bound,                                    /* min bound is copied into the search */
                                                 const vector<double> &max_bound,                                    /* max bound is copied into the search */
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
                                               ) throw (string) : DifferentialEvolution(min_bound, max_bound, population_size, parent_selection, number_pairs, recombination_selection, parent_scaling_factor, differential_scaling_factor, crossover_rate, directional, maximum_created, maximum_reported) {
    this->conn = conn;
    this->app_id = app_id;
    this->name = name;
    check_name(name);
    insert_to_database();
}

DifferentialEvolutionDB::~DifferentialEvolutionDB() {
    conn = NULL;
}

void
DifferentialEvolutionDB::new_individual(uint32_t &id, vector<double> &parameters, uint32_t &seed) throw (string) {
    DifferentialEvolution::new_individual(id, parameters, seed);
}

void
DifferentialEvolutionDB::new_individual(uint32_t &id, vector<double> &parameters) throw (string) {
    DifferentialEvolution::new_individual(id, parameters);
}


bool
DifferentialEvolutionDB::insert_individual(uint32_t id, const vector<double> &parameters, double fitness, uint32_t seed) throw (string) {
    bool modified = DifferentialEvolution::insert_individual(id, parameters, fitness);

    if (modified) {
        ostringstream individual_query;
        individual_query << "UPDATE de_individual"
                         << " SET "
                         << "  fitness = " << setprecision(10) << fitnesses[id]
                         << ", parameters = '" << vector_to_string<double>(population[id]) << "'"
                         << ", seed = " << seeds[id]
                         << " WHERE "
                         << "     differential_evolution_id = " << this->id
                         << " AND position = " << id;

        mysql_query(conn, individual_query.str().c_str());

//        cout << individual_query.str() << endl;

        if (mysql_errno(conn) != 0) {
            ostringstream ex_msg;
            ex_msg << "ERROR: updating individual with query: '" << individual_query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
            throw ex_msg.str();
        }

        ostringstream de_query;
        de_query << " UPDATE differential_evolution"
                 << " SET "
//                 << "  current_individual = " << current_individual             //probably should not have this here TODO: fix this for standard_benchmarks_db
                 << "  initialized_individuals = " << initialized_individuals
                 << ", current_iteration = " << current_iteration
//                 << ", maximum_iterations = " << maximum_iterations
//                 << ", individuals_created = " << individuals_created
//                 << ", maximum_created = " << maximum_created
                 << ", individuals_reported = " << individuals_reported
//                 << ", maximum_reported = " << maximum_reported
                 << " WHERE "
                 << "    id = " << this->id << endl;

        mysql_query(conn, de_query.str().c_str());

        if (mysql_errno(conn) != 0) {
            ostringstream ex_msg;
            ex_msg << "ERROR: updating differential_evolution with query: '" << individual_query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
            throw ex_msg.str();
        }


        double best, average, median, worst;
        calculate_fitness_statistics(fitnesses, best, average, median, worst);

        ostringstream log_query;
        log_query.precision(10);
        log_query << "INSERT INTO differential_evolution_log"
            << " SET "
            << "  search_id = " << this->id
            << ", evaluation = " << this->individuals_reported
            << ", current = '" << setprecision(10) << fitnesses[id] << "'"
            << ", best = '" << best << "'"
            << ", average = '" << average << "'"
            << ", median = '" << median << "'"
            << ", worst = '" << worst << "'"
            << ", individual = " << id
            << ", seed = " << seed
            << ", global = " << setprecision(10) << (fitnesses[id] == global_best_fitness);
//            << ", parameters = '" << vector_to_string<double>(parameters) << "'" << endl;

        mysql_query(conn, log_query.str().c_str());

        if (mysql_errno(conn) != 0) {
            ostringstream ex_msg;
            ex_msg << "ERROR: updating differential_evolution_log with query: '" << log_query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
            throw ex_msg.str();
        }


    }

    return modified;
}

void
DifferentialEvolutionDB::update_current_individual() throw (string) {
    ostringstream query;
    query << " UPDATE differential_evolution"
        << " SET "
        << "  current_individual = " << current_individual
        << ", current_iteration = " << current_iteration
        << ", individuals_created = " << individuals_created
        << " WHERE "
        << "    id = " << id << endl;

    mysql_query(conn, query.str().c_str());

    if (mysql_errno(conn) != 0) {
        ostringstream ex_msg;
        ex_msg << "ERROR: updating 'differential_evolution' with query: '" << query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }   
}

void
DifferentialEvolutionDB::add_searches(MYSQL *conn, int32_t app_id, vector<EvolutionaryAlgorithmDB*> &searches) throw (string) {
    ostringstream query;
    query << "SELECT id FROM differential_evolution WHERE app_id = " << app_id;

    mysql_query(conn, query.str().c_str());
    MYSQL_RES *result = mysql_store_result(conn);

    if (mysql_errno(conn) != 0) {
        ostringstream ex_msg;
        ex_msg << "ERROR: getting unfinished searches with query: '" << query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }   

    MYSQL_ROW individual_row;

    while ((individual_row = mysql_fetch_row(result))) {
        if (mysql_errno(conn) != 0) {
            ostringstream ex_msg;
            ex_msg << "ERROR: getting row for unfinished searches with query: '" << query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
            throw ex_msg.str();
        }   

        searches.push_back(new DifferentialEvolutionDB(conn, atoi(individual_row[0])));
    }   
    mysql_free_result(result);
}


/** TODO: remove this and add an 'remote_finished_searches'/'remove_finished_searches' method to EvolutionaryAlgorithmBM **/
void
DifferentialEvolutionDB::add_unfinished_searches(MYSQL *conn, int32_t app_id, vector<EvolutionaryAlgorithmDB*> &unfinished_searches) throw (string) {
    ostringstream query;
    query << "SELECT id FROM differential_evolution WHERE app_id = " << app_id;

    mysql_query(conn, query.str().c_str());
    MYSQL_RES *result = mysql_store_result(conn);

    if (mysql_errno(conn) != 0) {
        ostringstream ex_msg;
        ex_msg << "ERROR: getting unfinished searches with query: '" << query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }   

    MYSQL_ROW individual_row;

    while ((individual_row = mysql_fetch_row(result))) {
        if (mysql_errno(conn) != 0) {
            ostringstream ex_msg;
            ex_msg << "ERROR: getting row for unfinished searches with query: '" << query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
            throw ex_msg.str();
        }   

        DifferentialEvolutionDB *search = new DifferentialEvolutionDB(conn, atoi(individual_row[0]));

        if (search->is_running()) {
            unfinished_searches.push_back(search);
        } else {
            delete search;
        }   
    }   
    mysql_free_result(result);
}

void
DifferentialEvolutionDB::print_to(ostream& stream) {
    stream  << "[DifferentialEvolutionDB " << endl
            << "    id = " << id << endl
            << "    name = '" << name << "'" << endl
            << "    parent_selection = " << parent_selection
            << "    number_pairs = " << number_pairs
            << "    recombination_selection = " << recombination_selection
            << "    parent_scaling_factor = " << parent_scaling_factor
            << "    differential_scaling_factor = " << differential_scaling_factor
            << "    crossover_rate = " << crossover_rate
            << "    directional = " << directional
            << "    current_individual = " << current_individual << endl
            << "    initialized_individuals = " << initialized_individuals << endl
            << "    current_iteration = " << current_iteration << endl
            << "    maximum_iterations = " << maximum_iterations << endl
            << "    individuals_created = " << individuals_created << endl
            << "    maximum_created = " << maximum_created << endl
            << "    individuals_reported = " << individuals_reported << endl
            << "    maximum_reported = " << maximum_reported << endl
            << "    population_size = " << population_size << endl
            << "    min_bound = '" << vector_to_string<double>(min_bound) << "'" << endl
            << "    max_bound = '" << vector_to_string<double>(max_bound) << "'" << endl
            << "    app_id = " << app_id << endl
            << "]" << endl;

    for (uint32_t i = 0; i < population_size; i++) {
        stream << "    [DEIndividual" << endl
               << "        differential_evolution_id = " << id << endl
               << "        position = " << i << endl
               << "        fitness = " << setprecision(10) << fitnesses[i] << endl
               << "        parameters = '" << vector_to_string<double>(population[i]) << "'" << endl
               << "        seed = " << seeds[i] << endl
               << "    ]" << endl;
    }
}

ostream& operator<< (ostream& stream, DifferentialEvolutionDB &ps) {
    ps.print_to(stream);
    return stream;
}
