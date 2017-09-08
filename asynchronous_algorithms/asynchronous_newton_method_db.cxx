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

#include "evolutionary_algorithm_db.hxx"
#include "asynchronous_newton_method_db.hxx"

#include "util/arguments.hxx"
#include "util/vector_io.hxx"

/**
 *  From MYSQL
 */
#include "mysql.h"

using namespace std;

void
AsynchronousNewtonMethodDB::check_name(string name) throw (string) {
    if (name.substr(0,4).compare("anm_") != 0) {
        ostringstream err_msg;
        err_msg << "Improper name for AsynchronousNewtonMethodDB '" << name << "' must start with 'anm_'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw err_msg.str();
    }
}

/**
 *  The following construct a AsynchronousNewtonMethod from a database entry
 */
AsynchronousNewtonMethodDB::AsynchronousNewtonMethodDB(MYSQL *conn, string name) throw (string) {
    check_name(name);
    this->conn = conn;

    ostringstream oss;
    oss << "SELECT * FROM asynchronous_newton_method WHERE name = '" << name << "'";
//    cout << oss.str() << endl;

    construct_from_database(oss.str());
}

AsynchronousNewtonMethodDB::AsynchronousNewtonMethodDB(MYSQL *conn, int id) throw (string) {
    this->conn = conn;

    ostringstream oss;
    oss << "SELECT * FROM asynchronous_newton_method WHERE id = " << id;
//    cout << oss.str() << endl;

    construct_from_database(oss.str());
}

bool
AsynchronousNewtonMethodDB::search_exists(MYSQL *conn, string search_name) throw (string) {
    ostringstream query;
    query << "SELECT id FROM asynchronous_newton_method where name = '" << search_name << "'";

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
AsynchronousNewtonMethodDB::create_tables(MYSQL *conn) throw (string) {
    ostringstream anm_query;
    anm_query << "CREATE TABLE `asynchronous_newton_method` ("
                << "    `id` int(11) NOT NULL AUTO_INCREMENT,"
                << "    `name` varchar(254) NOT NULL DEFAULT '',"
                << "    `regression_radius` varchar(2048) NOT NULL,"
                << "    `center` varchar(2048) NOT NULL,"
                << "    `center_fitness` double NOT NULL,"
                << "    `line_search_direction` varchar(2048) NOT NULL,"
                << "    `line_search_min` double NOT NULL DEFAULT '-1',"
                << "    `line_search_max` double NOT NULL DEFAULT '2',"
                << "    `extra_workunits` int(11) NOT NULL DEFAULT '0', "
                << "    `minimum_line_search_individuals` int(11) NOT NULL DEFAULT '0', "
                << "    `minimum_regression_individuals` int(11) NOT NULL DEFAULT '0', "
                << "    `line_search_individuals_reported` int(11) NOT NULL DEFAULT '0', "
                << "    `regression_individuals_reported` int(11) NOT NULL DEFAULT '0', "
                << "    `current_iteration` int(11) NOT NULL DEFAULT '0',"
                << "    `maximum_iterations` int(11) NOT NULL DEFAULT '0',"
                << "    `first_workunits_generated` tinyint(1) NOT NULL DEFAULT '0',"
                << "    `min_bound` varchar(2048) NOT NULL,"
                << "    `max_bound` varchar(2048) NOT NULL,"
                << "    `app_id`    int(11) NOT NULL DEFAULT '-1',"
                << "PRIMARY KEY (`id`),"
                << "UNIQUE KEY `name` (`name`)"
                << ") ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=latin1";

    cout << "creating asynchronous_newton_method table with: " << endl << anm_query.str() << endl << endl;

    if (mysql_query(conn, anm_query.str().c_str())) {
        ostringstream ex_msg;
        ex_msg << "ERROR: could not create asynchronous newton method table with query: '" << anm_query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }

    ostringstream line_search_query;
    line_search_query << "CREATE TABLE `anm_line_search` ("
                      << "    `asynchronous_newton_method_id` int(11) NOT NULL,"
                      << "    `position` int(11) NOT NULL,"
                      << "    `fitness` double NOT NULL,"
                      << "    `parameters` varchar(2048) NOT NULL,"
                      << "    `seed` int(32) UNSIGNED,"
                      << "PRIMARY KEY (`asynchronous_newton_method_id`,`position`)"
                      << ") ENGINE=InnoDB DEFAULT CHARSET=latin1";

    cout << "creating anm_line_search table with: " << endl << line_search_query.str() << endl << endl;

    if (mysql_query(conn, line_search_query.str().c_str())) {
        ostringstream ex_msg;
        ex_msg << "ERROR: could not create anm_line_search table with query: '" << line_search_query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }

    ostringstream regression_query;
    regression_query  << "CREATE TABLE `anm_regression` ("
                      << "    `asynchronous_newton_method_id` int(11) NOT NULL,"
                      << "    `position` int(11) NOT NULL,"
                      << "    `fitness` double NOT NULL,"
                      << "    `parameters` varchar(2048) NOT NULL,"
                      << "    `seed` int(32) UNSIGNED,"
                      << "PRIMARY KEY (`asynchronous_newton_method_id`,`position`)"
                      << ") ENGINE=InnoDB DEFAULT CHARSET=latin1";

    cout << "creating anm_regression table with: " << endl << regression_query.str() << endl << endl;

    if (mysql_query(conn, regression_query.str().c_str())) {
        ostringstream ex_msg;
        ex_msg << "ERROR: could not create anm_regression table with query: '" << regression_query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }
}

void 
AsynchronousNewtonMethodDB::construct_from_database(string query) throw (string) {
    mysql_query(conn, query.c_str());

    if (mysql_errno(conn) != 0) {
        ostringstream ex_msg;
        ex_msg << "ERROR: could not get asynchronous newton method from query: '" << query << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }

    MYSQL_RES *result = mysql_store_result(conn);

    if (result != NULL) {
        MYSQL_ROW row = mysql_fetch_row(result);
        
        if (row == NULL) {
            ostringstream ex_msg;
            ex_msg << "ERROR: could not construct asynchronous newton method '" << name << "' from database, it does not exist. Thrown on " << __FILE__ << ":" << __LINE__;
            throw ex_msg.str();
        }

        construct_from_database(row);
        mysql_free_result(result);
    } else {
        ostringstream ex_msg;
        ex_msg << "ERROR: could not get asynchronous newton method from query: '" << query << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }
}


void 
AsynchronousNewtonMethodDB::construct_from_database(MYSQL_ROW row) throw (string) {
    id = atoi(row[0]);
    name = row[1];
    string_to_vector<double>(row[2], regression_radius);
    string_to_vector<double>(row[3], center);
    center_fitness = atof(row[4]);
    string_to_vector<double>(row[5], line_search_direction);
    line_search_min = atof(row[6]);
    line_search_max = atof(row[7]);
    extra_workunits = atoi(row[8]);
    minimum_line_search_individuals = atoi(row[9]);
    minimum_regression_individuals = atoi(row[10]);
    line_search_individuals_reported = atoi(row[11]);
    regression_individuals_reported = atoi(row[12]);
    current_iteration = atoi(row[13]);
    maximum_iterations = atoi(row[14]);
    first_workunits_generated = atoi(row[15]);
    string_to_vector<double>(row[16], min_bound);
    string_to_vector<double>(row[17], max_bound);
    app_id = atoi(row[18]);
    number_parameters = min_bound.size();

    //Get the individual information from the database
    ostringstream oss;
    oss << "SELECT position, fitness, parameters, seed FROM anm_line_search WHERE asynchronous_newton_method_id = " << this->id << " ORDER BY position";
    mysql_query(conn, oss.str().c_str());
    MYSQL_RES *result = mysql_store_result(conn);

//    cout << oss.str() << endl;

    uint32_t number_individuals = minimum_line_search_individuals + extra_workunits;
    line_search_fitnesses.resize(number_individuals, -numeric_limits<double>::max());
    line_search_individuals.resize(number_individuals, vector<double>(number_parameters, 0.0));
    line_search_seeds.resize(number_individuals, 0);

    AsynchronousNewtonMethod::initialize_rng();    //to initialize the random number generator

    if (result != NULL) {
        uint32_t num_results = mysql_num_rows(result);
        if (num_results != number_individuals) {
            ostringstream ex_msg;
            ex_msg << "ERROR: got " << num_results << " results when looking up individuals for search " << name << ", with a population size: " << number_individuals << ". Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
            throw ex_msg.str();
        }   

        MYSQL_ROW individual_row;

        while ((individual_row = mysql_fetch_row(result))) {
            int individual_id = atoi(individual_row[0]);
            line_search_fitnesses[individual_id] = atof(individual_row[1]);

            if (line_search_fitnesses[individual_id] < -1.79768e+308) {
                line_search_fitnesses[individual_id] = -numeric_limits<double>::max();
            }

            string_to_vector<double>(individual_row[2], line_search_individuals[individual_id]);
            line_search_seeds[individual_id] = atoi(individual_row[3]);

//            cout   << "    [anm_line_search individual" << endl
//                   << "        position = " << individual_id << endl
//                   << "        fitness = " << fitnesses[individual_id] << endl
//                   << "        parameters = '" << vector_to_string<double>(population[individual_id]) << "'" << endl
//                   << "    ]" << endl;

         }   
        mysql_free_result(result);
    } else {
        ostringstream ex_msg;
        ex_msg << "ERROR: looking up line search individuals with query: '" << oss.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }

    //Get the individual information from the database
    oss.clear();
    oss.str("");
    oss << "SELECT position, fitness, parameters, seed FROM anm_regression WHERE asynchronous_newton_method_id = " << this->id << " ORDER BY position";
    mysql_query(conn, oss.str().c_str());
    result = mysql_store_result(conn);

//    cout << oss.str() << endl;

    number_individuals = minimum_regression_individuals + extra_workunits;
    regression_fitnesses.resize(number_individuals, -numeric_limits<double>::max());
    regression_individuals.resize(number_individuals, vector<double>(number_parameters, 0.0));
    regression_seeds.resize(number_individuals, 0);

    AsynchronousNewtonMethod::initialize_rng();    //to initialize the random number generator

    if (result != NULL) {
        uint32_t num_results = mysql_num_rows(result);
        if (num_results != number_individuals) {
            ostringstream ex_msg;
            ex_msg << "ERROR: got " << num_results << " results when looking up individuals for search " << name << ", with a population size: " << number_individuals << ". Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
            throw ex_msg.str();
        }   

        MYSQL_ROW individual_row;

        while ((individual_row = mysql_fetch_row(result))) {
            int individual_id = atoi(individual_row[0]);
            regression_fitnesses[individual_id] = atof(individual_row[1]);

            if (regression_fitnesses[individual_id] < -1.79768e+308) {
                regression_fitnesses[individual_id] = -numeric_limits<double>::max();
            }

            string_to_vector<double>(individual_row[2], regression_individuals[individual_id]);
            regression_seeds[individual_id] = atoi(individual_row[3]);

//            cout   << "    [anm_regression individual" << endl
//                   << "        position = " << individual_id << endl
//                   << "        fitness = " << fitnesses[individual_id] << endl
//                   << "        parameters = '" << vector_to_string<double>(population[individual_id]) << "'" << endl
//                   << "    ]" << endl;

         }   
        mysql_free_result(result);
    } else {
        ostringstream ex_msg;
        ex_msg << "ERROR: looking up regression individuals with query: '" << oss.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }

}


/**
 *  Insert a created asynchronous newton method to a database.
 */
void
AsynchronousNewtonMethodDB::insert_to_database() throw (string) {
    ostringstream query;

    query << "INSERT INTO asynchronous_newton_method"
          << " SET "
          << "  name = '" << name << "'"
          << ", regression_radius = '" << vector_to_string<double>(regression_radius) << "'"
          << ", center = '" << vector_to_string<double>(regression_radius) << "'"
          << ", center_fitness = " << center_fitness 
          << ", line_search_direction = '" << vector_to_string<double>(line_search_direction) << "'"
          << ", line_search_min = " << line_search_min
          << ", line_search_max = " << line_search_max 
          << ", extra_workunits = " << extra_workunits 
          << ", minimum_line_search_individuals = " << minimum_line_search_individuals
          << ", minimum_regression_individuals = " << minimum_regression_individuals 
          << ", line_search_individuals_reported = " << line_search_individuals_reported
          << ", regression_individuals_reported = " << regression_individuals_reported
          << ", current_iteration = " << current_iteration
          << ", maximum_iterations = " << maximum_iterations 
          << ", first_workunits_generated = " << first_workunits_generated
          << ", min_bound = '" << vector_to_string<double>(min_bound) << "'"
          << ", max_bound = '" << vector_to_string<double>(max_bound) << "'"
          << ", app_id = " << app_id;

    mysql_query(conn, query.str().c_str());

    MYSQL_RES *result;
    if ((result = mysql_store_result(conn)) == 0 && mysql_field_count(conn) == 0 && mysql_insert_id(conn) != 0) {
        id = mysql_insert_id(conn);
    } else {
        ostringstream ex_msg;
        ex_msg << "ERROR: could not get asynchronous newton method id from insert query: '" << query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }
    mysql_free_result(result);

    for (uint32_t i = 0; i < minimum_line_search_individuals + extra_workunits; i++) {
        ostringstream individual_query;
        individual_query << "INSERT INTO anm_line_search"
                         << " SET "
                         << "  asynchronous_newton_method_id = " << id
                         << ", position = " << i
                         << ", fitness = " << line_search_fitnesses[i]
                         << ", parameters = '" << vector_to_string<double>(line_search_individuals[i]) << "'"
                         << ", seed = " << line_search_seeds[i];

        mysql_query(conn, individual_query.str().c_str());

        if (mysql_errno(conn) != 0) {
            ostringstream ex_msg;
            ex_msg << "ERROR: creating anm_line_search with insert query: '" << individual_query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
            throw ex_msg.str();
        }
        mysql_free_result(result);
    }

    for (uint32_t i = 0; i < minimum_regression_individuals + extra_workunits; i++) {
        ostringstream individual_query;
        individual_query << "INSERT INTO anm_regression"
                         << " SET "
                         << "  asynchronous_newton_method_id = " << id
                         << ", position = " << i
                         << ", fitness = " << regression_fitnesses[i]
                         << ", parameters = '" << vector_to_string<double>(regression_individuals[i]) << "'"
                         << ", seed = " << regression_seeds[i];

        mysql_query(conn, individual_query.str().c_str());

        if (mysql_errno(conn) != 0) {
            ostringstream ex_msg;
            ex_msg << "ERROR: creating anm_regression with insert query: '" << individual_query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
            throw ex_msg.str();
        }
        mysql_free_result(result);
    }
}

/**
 *  The following constructors create new AsynchronousNewtonMethodDBs and insert them into the database.
 */
AsynchronousNewtonMethodDB::AsynchronousNewtonMethodDB(
                                MYSQL *conn,
                                const vector<string> &arguments
                            ) throw (string) : AsynchronousNewtonMethod(arguments) {
    this->conn = conn;
    this->app_id = -1;
    get_argument(arguments, "--search_name", true, name);
    check_name(name);
    insert_to_database();
}

AsynchronousNewtonMethodDB::AsynchronousNewtonMethodDB(
                                MYSQL *conn,
                                const int32_t app_id,
                                const vector<string> &arguments
                            ) throw (string) : AsynchronousNewtonMethod(arguments) {
    this->conn = conn;
    this->app_id = app_id;
    get_argument(arguments, "--search_name", true, name);
    check_name(name);
    insert_to_database();
}


AsynchronousNewtonMethodDB::AsynchronousNewtonMethodDB(
                                MYSQL *conn,
                                const vector<double> &min_bound,
                                const vector<double> &max_bound,
                                const vector<double> &regression_width,
                                const vector<string> &arguments
                            ) throw (string) : AsynchronousNewtonMethod(min_bound, max_bound, regression_width, arguments) {
    this->conn = conn;
    this->app_id = -1;
    get_argument(arguments, "--search_name", true, name);
    check_name(name);
    insert_to_database();
}

AsynchronousNewtonMethodDB::AsynchronousNewtonMethodDB(
                                MYSQL *conn,
                                const int32_t app_id,
                                const vector<double> &min_bound,
                                const vector<double> &max_bound,
                                const vector<double> &regression_width,
                                const vector<string> &arguments
                            ) throw (string) : AsynchronousNewtonMethod(min_bound, max_bound, regression_width, arguments) {
    this->conn = conn;
    this->app_id = app_id;
    get_argument(arguments, "--search_name", true, name);
    check_name(name);
    insert_to_database();
}

AsynchronousNewtonMethodDB::AsynchronousNewtonMethodDB(
                                MYSQL *conn,
                                const vector<double> &min_bound,
                                const vector<double> &max_bound,
                                const vector<double> &current_center,
                                const vector<double> &regression_width,
                                const vector<string> &arguments
                            ) throw (string) : AsynchronousNewtonMethod(min_bound, max_bound, current_center, regression_width, arguments) {
    this->conn = conn;
    this->app_id = -1;
    get_argument(arguments, "--search_name", true, name);
    check_name(name);
    insert_to_database();
}

AsynchronousNewtonMethodDB::AsynchronousNewtonMethodDB(
                                MYSQL *conn,
                                const int32_t app_id,
                                const vector<double> &min_bound,
                                const vector<double> &max_bound,
                                const vector<double> &current_center,
                                const vector<double> &regression_width,
                                const vector<string> &arguments
                            ) throw (string) : AsynchronousNewtonMethod(min_bound, max_bound, current_center, regression_width, arguments) {
    this->conn = conn;
    this->app_id = app_id;
    get_argument(arguments, "--search_name", true, name);
    check_name(name);
    insert_to_database();
}

AsynchronousNewtonMethodDB::AsynchronousNewtonMethodDB(
                                MYSQL *conn,
                                string name, 
                                const vector<double> &min_bound,                  /* min bound is copied into the search */
                                const vector<double> &max_bound,                  /* max bound is copied into the search */
                                const vector<double> &current_center,
                                const vector<double> &regression_width,
                                const uint32_t minimum_regression_individuals,
                                const uint32_t minimum_line_search_individuals,
                                const uint32_t maximum_iterations                 /* default value is 0 which means no termination */
                            ) throw (string) : AsynchronousNewtonMethod(min_bound, max_bound, current_center, regression_width, minimum_regression_individuals, minimum_line_search_individuals, maximum_iterations) {
    this->conn = conn;
    this->app_id = -1;
    this->name = name;
    check_name(name);
    insert_to_database();
}

AsynchronousNewtonMethodDB::AsynchronousNewtonMethodDB(
                                MYSQL *conn,
                                const int32_t app_id,
                                string name,
                                const vector<double> &min_bound,                  /* min bound is copied into the search */
                                const vector<double> &max_bound,                  /* max bound is copied into the search */
                                const vector<double> &current_center,
                                const vector<double> &regression_width,
                                const uint32_t minimum_regression_individuals,
                                const uint32_t minimum_line_search_individuals,
                                const uint32_t maximum_iterations                 /* default value is 0 which means no termination */
                            ) throw (string) : AsynchronousNewtonMethod(min_bound, max_bound, current_center, regression_width, minimum_regression_individuals, minimum_line_search_individuals, maximum_iterations) {
    this->conn = conn;
    this->app_id = app_id;
    this->name = name;
    check_name(name);
    insert_to_database();
}

AsynchronousNewtonMethodDB::~AsynchronousNewtonMethodDB() {
    conn = NULL;
}

void
AsynchronousNewtonMethodDB::update_database_on_generate() throw (string) {
    ostringstream individual_query;
    individual_query     << "UPDATE asynchronous_newton_method"
                         << " SET ";
    if (current_iteration == 0) {
        individual_query << "  first_workunits_generated = 1,";
    }
    individual_query     << "  current_iteration = " << current_iteration
                         << ", line_search_individuals_reported = " << line_search_individuals_reported
                         << ", regression_individuals_reported = " << line_search_individuals_reported
                         << ", center_fitness = " << center_fitness
                         << ", center = '" << vector_to_string<double>(center) << "'"
                         << ", line_search_direction = '" << vector_to_string<double>(line_search_direction) << "'"
                         << " WHERE "
                         << "     id = " << this->id;

    mysql_query(conn, individual_query.str().c_str());

    if (mysql_errno(conn) != 0) {
        ostringstream ex_msg;
        ex_msg << "ERROR: updating individual with query: '" << individual_query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }
}

bool
AsynchronousNewtonMethodDB::generate_individuals(uint32_t &number_individuals, uint32_t &iteration, vector< vector<double> > &parameters) throw (string) {
    bool modified = AsynchronousNewtonMethod::generate_individuals(number_individuals, iteration, parameters);
    if (modified) update_database_on_generate();
    return modified;
}

bool
AsynchronousNewtonMethodDB::generate_individuals(uint32_t &number_individuals, uint32_t &iteration, vector< vector<double> > &parameters, vector<uint32_t> &seeds) throw (string) {
    bool modified = AsynchronousNewtonMethod::generate_individuals(number_individuals, iteration, parameters, seeds);
    if (modified) update_database_on_generate();
    return modified;
}

void 
AsynchronousNewtonMethodDB::update_database_on_insert(uint32_t id, const vector<double> &parameters, double fitness, bool using_seed, uint32_t seed) throw (string) {
    ostringstream individual_query;
    if (current_iteration % 2 == 0) {
        individual_query << "UPDATE anm_regression"
                         << " SET "
                         << "  fitness = " << regression_fitnesses[id]
                         << ", parameters = '" << vector_to_string<double>(regression_individuals[id]) << "'";
        if (using_seed) {
        individual_query << ", seed = " << regression_seeds[id];
        }
        individual_query << " WHERE "
                         << "     asynchronous_newton_method_id = " << this->id
                         << " AND position = " << id;
    } else {
        individual_query << "UPDATE anm_line_search"
                         << " SET "
                         << "  fitness = " << line_search_fitnesses[id]
                         << ", parameters = '" << vector_to_string<double>(line_search_individuals[id]) << "'";
        if (using_seed) {
        individual_query << ", seed = " << line_search_seeds[id];
        }
        individual_query << " WHERE "
                         << "     asynchronous_newton_method_id = " << this->id
                         << " AND position = " << id;
    }

    mysql_query(conn, individual_query.str().c_str());

    if (mysql_errno(conn) != 0) {
        ostringstream ex_msg;
        ex_msg << "ERROR: updating individual with query: '" << individual_query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }

    if (current_iteration % 2 == 0) {
        ostringstream anm_query;
        anm_query << " UPDATE asynchronous_newton_method"
                  << " SET "
                  << "  regression_individuals_reported = " << regression_individuals_reported 
                  << " WHERE "
                  << "    id = " << this->id << endl;

        mysql_query(conn, anm_query.str().c_str());
    } else {
        ostringstream anm_query;
        anm_query << " UPDATE asynchronous_newton_method"
                  << " SET "
                  << "  line_search_individuals_reported = " << line_search_individuals_reported 
                  << " WHERE "
                  << "    id = " << this->id << endl;

        mysql_query(conn, anm_query.str().c_str());
    }

    if (mysql_errno(conn) != 0) {
        ostringstream ex_msg;
        ex_msg << "ERROR: updating asynchronous_newton_method with query: '" << individual_query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }
}

bool
AsynchronousNewtonMethodDB::insert_individual(uint32_t id, const vector<double> &parameters, double fitness) throw (string) {
    bool modified = AsynchronousNewtonMethod::insert_individual(id, parameters, fitness);
    if (modified) update_database_on_insert(id, parameters, fitness, true, 0);
    return modified;
}


bool
AsynchronousNewtonMethodDB::insert_individual(uint32_t id, const vector<double> &parameters, double fitness, uint32_t seed) throw (string) {
    bool modified = AsynchronousNewtonMethod::insert_individual(id, parameters, fitness, seed);
    if (modified) update_database_on_insert(id, parameters, fitness, true, seed);
    return modified;
}

void
AsynchronousNewtonMethodDB::add_searches(MYSQL *conn, int32_t app_id, vector<AsynchronousNewtonMethodDB*> &searches) throw (string) {
    ostringstream query;
    query << "SELECT id FROM asynchronous_newton_method WHERE app_id = " << app_id;

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

        searches.push_back(new AsynchronousNewtonMethodDB(conn, atoi(individual_row[0])));
    }   
    mysql_free_result(result);
}


/** TODO: remove this and add an 'remove_finished_searches'/'remove_finished_searches' method to AsynchronousNewtonMethodDB **/
void
AsynchronousNewtonMethodDB::add_unfinished_searches(MYSQL *conn, int32_t app_id, vector<AsynchronousNewtonMethodDB*> &unfinished_searches) throw (string) {
    ostringstream query;
    query << "SELECT id FROM asynchronous_newton_method WHERE app_id = " << app_id;

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

        AsynchronousNewtonMethodDB *search = new AsynchronousNewtonMethodDB(conn, atoi(individual_row[0]));

        if (search->is_running()) {
            unfinished_searches.push_back(search);
        } else {
            delete search;
        }   
    }   
    mysql_free_result(result);
}

bool
AsynchronousNewtonMethodDB::is_running() {
        return (maximum_iterations == 0 || current_iteration < maximum_iterations) && (max_failed_improvements == 0 || failed_improvements < max_failed_improvements);
}

void
AsynchronousNewtonMethodDB::print_to(ostream& stream) {
    stream  << "[AsynchronousNewtonMethodDB " << endl
            << "  name = '" << name << "'"
            << ", regression_radius = '" << vector_to_string<double>(regression_radius) << "'"
            << ", center = '" << vector_to_string<double>(regression_radius) << "'"
            << ", center_fitness = " << center_fitness 
            << ", line_search_direction = '" << vector_to_string<double>(line_search_direction) << "'"
            << ", line_search_min = " << line_search_min
            << ", line_search_max = " << line_search_max 
            << ", extra_workunits = " << extra_workunits 
            << ", minimum_line_search_individuals = " << minimum_line_search_individuals 
            << ", minimum_regression_individuals = " << minimum_regression_individuals
            << ", line_search_individuals_reported = " << line_search_individuals_reported
            << ", regression_individuals_reported = " << regression_individuals_reported
            << ", current_iteration = " << current_iteration
            << ", maximum_iterations = " << maximum_iterations
            << ", first_workunits_generated = " << first_workunits_generated
            << ", min_bound = '" << vector_to_string<double>(min_bound) << "'"
            << ", max_bound = '" << vector_to_string<double>(max_bound) << "'"
            << ", app_id = " << app_id
            << "]" << endl;

    uint32_t number_individuals = minimum_line_search_individuals + extra_workunits;
    for (uint32_t i = 0; i < number_individuals ; i++) {
        stream << "    [ANM Line Search Individual" << endl
               << "        asynchronous_newton_method_id = " << id << endl
               << "        position = " << i << endl
               << "        fitness = " << line_search_fitnesses[i] << endl
               << "        parameters = '" << vector_to_string<double>(line_search_individuals[i]) << "'" << endl
               << "        seed = " << line_search_seeds[i] << endl
               << "    ]" << endl;
    }

    number_individuals = minimum_regression_individuals + extra_workunits;
    for (uint32_t i = 0; i < number_individuals ; i++) {
        stream << "    [ANM Regression Individual" << endl
               << "        asynchronous_newton_method_id = " << id << endl
               << "        position = " << i << endl
               << "        fitness = " << regression_fitnesses[i] << endl
               << "        parameters = '" << vector_to_string<double>(regression_individuals[i]) << "'" << endl
               << "        seed = " << regression_seeds[i] << endl
               << "    ]" << endl;
    }
}

ostream& operator<< (ostream& stream, AsynchronousNewtonMethodDB &ps) {
    ps.print_to(stream);
    return stream;
}
