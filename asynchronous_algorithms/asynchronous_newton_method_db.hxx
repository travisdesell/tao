#ifndef TAO_ASYNCHRONOUS_NEWTON_METHOD_DB_H
#define TAO_ASYNCHRONOUS_NEWTON_METHOD_DB_H

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

#include "asynchronous_algorithms/asynchronous_newton_method.hxx"

using namespace std;

class AsynchronousNewtonMethodDB : public AsynchronousNewtonMethod {
    protected:
        int id;
        string name;
        int app_id;

        MYSQL *conn;

        void check_name(string name) throw (string);

        AsynchronousNewtonMethodDB();
    public:
        ~AsynchronousNewtonMethodDB();

        AsynchronousNewtonMethodDB(MYSQL *conn, string name) throw (string);
        AsynchronousNewtonMethodDB(MYSQL *conn, int id) throw (string);

        AsynchronousNewtonMethodDB(
                                MYSQL *conn,
                                const vector<string> &arguments
                            ) throw (string);

        AsynchronousNewtonMethodDB(
                                MYSQL *conn,
                                const int32_t app_id,
                                const vector<string> &arguments
                            ) throw (string);

        AsynchronousNewtonMethodDB(
                                MYSQL *conn,
                                const vector<double> &min_bound,
                                const vector<double> &max_bound,
                                const vector<double> &regression_width,
                                const vector<string> &arguments
                            ) throw (string);

        AsynchronousNewtonMethodDB(
                                MYSQL *conn,
                                const int32_t app_id,
                                const vector<double> &min_bound,
                                const vector<double> &max_bound,
                                const vector<double> &regression_width,
                                const vector<string> &arguments
                            ) throw (string);

        AsynchronousNewtonMethodDB(
                                MYSQL *conn,
                                const vector<double> &min_bound,
                                const vector<double> &max_bound,
                                const vector<double> &current_center,
                                const vector<double> &regression_width,
                                const vector<string> &arguments
                            ) throw (string);


        AsynchronousNewtonMethodDB(
                                MYSQL *conn,
                                const int32_t app_id,
                                const vector<double> &min_bound,
                                const vector<double> &max_bound,
                                const vector<double> &current_center,
                                const vector<double> &regression_width,
                                const vector<string> &arguments
                            ) throw (string);

        AsynchronousNewtonMethodDB(
                                MYSQL *conn,
                                string name,
                                const vector<double> &min_bound,                  /* min bound is copied into the search */
                                const vector<double> &max_bound,                  /* max bound is copied into the search */
                                const vector<double> &current_center,
                                const vector<double> &regression_width,
                                const uint32_t minimum_regression_individuals,
                                const uint32_t minimum_line_search_individuals,
                                const uint32_t maximum_iterations                 /* default value is 0 which means no termination */
                            ) throw (string);

        AsynchronousNewtonMethodDB(
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
                            ) throw (string);

        bool is_running();

        void update_database_on_generate() throw (string);
        void update_database_on_insert(uint32_t id, const vector<double> &parameters, double fitness, bool using_seed, uint32_t seed) throw (string);

        static bool search_exists(MYSQL *conn, std::string search_name) throw (std::string);
        static void create_tables(MYSQL *conn) throw (std::string);

        void construct_from_database(std::string query) throw (std::string);
        void construct_from_database(MYSQL_ROW row) throw (std::string);
        void insert_to_database() throw (std::string);           /* Insert a particle swarm into the database */

        static void add_searches(MYSQL *conn, int32_t app_id, std::vector<AsynchronousNewtonMethodDB*> &searches) throw (std::string);
        static void add_unfinished_searches(MYSQL *conn, int32_t app_id, std::vector<AsynchronousNewtonMethodDB*> &unfinished_searches) throw (std::string);

        void print_to(std::ostream& stream);
        friend std::ostream& operator<< (std::ostream& stream, AsynchronousNewtonMethodDB &ps);

        //returns true if it generates individuals
        virtual bool generate_individuals(uint32_t &number_individuals, uint32_t &iteration, vector< vector<double> > &parameters, vector<uint32_t> &seeds) throw (string);
        virtual bool generate_individuals(uint32_t &number_individuals, uint32_t &iteration, vector< vector<double> > &parameters) throw (string);

        //returns true if modified
        virtual bool insert_individual(uint32_t iteration, const vector<double> &parameters, double fitness, uint32_t seed) throw (string);
        virtual bool insert_individual(uint32_t iteration, const vector<double> &parameters, double fitness) throw (string);
};

#endif
