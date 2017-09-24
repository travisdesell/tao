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

#include <cmath>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <cfloat>
#include <string>
#include <stdint.h>

#include "examples/benchmarks.hxx"

#include "asynchronous_algorithms/particle_swarm.hxx"
#include "asynchronous_algorithms/particle_swarm_db.hxx"
#include "asynchronous_algorithms/differential_evolution.hxx"
#include "asynchronous_algorithms/differential_evolution_db.hxx"
#include "asynchronous_algorithms/asynchronous_newton_method.hxx"
#include "asynchronous_algorithms/asynchronous_newton_method_db.hxx"

#include "util/arguments.hxx"

#include "mysql.h"

/**
 *  Define a type for our objective function so we
 *  can pick which one to use and then the rest of
 *  the code will be the same.
 *
 *  Note that all the objective functions return a
 *  double, and take a double* and uint32_t as arguments.
 */
typedef double (*objective_function)(const vector<double> &);

int main(int argc /* number of command line arguments */, char **argv /* command line argumens */ ) {
    vector<string> arguments(argv, argv + argc);

    //assign the objective function variable to the objective function we're going to use
    string objective_function_name;
    get_argument(arguments, "--objective_function", true, objective_function_name);

    objective_function f = NULL;
    //compare returns 0 if the two strings are the same
    if (objective_function_name.compare("sphere") == 0)             f = sphere;
    else if (objective_function_name.compare("ackley") == 0)        f = ackley;
    else if (objective_function_name.compare("griewank") == 0)      f = griewank;
    else if (objective_function_name.compare("rastrigin") == 0)     f = rastrigin;
    else if (objective_function_name.compare("rosenbrock") == 0)    f = rosenbrock;
    else {
        cerr << "Improperly specified objective function: '" << objective_function_name.c_str() << "'" << endl;
        cerr << "Possibilities are:" << endl;
        cerr << "    sphere" << endl;
        cerr << "    ackley" << endl;
        cerr << "    griewank" << endl;
        cerr << "    rastrigin" << endl;
        cerr << "    rosenbrock" << endl;
        exit(1);
    }

    /**
     *  Initialize the arrays for the minimum and maximum bounds of the search space.
     *  These are different for the different objective functions.
     */
    uint32_t number_of_parameters;
    get_argument(arguments, "--n_parameters", true, number_of_parameters);
    vector<double> min_bound(number_of_parameters, 0);
    vector<double> max_bound(number_of_parameters, 0);
    vector<double> radius(number_of_parameters, 0);

    for (uint32_t i = 0; i < number_of_parameters; i++) {        //arrays go from 0 to size - 1 (not 1 to size)
        radius[i] = 0.2;
        if (objective_function_name.compare("sphere") == 0) {
            min_bound[i] = -100;
            max_bound[i] = 100;
        } else if (objective_function_name.compare("ackley") == 0) {
            min_bound[i] = -32;
            max_bound[i] = 32;
        } else if (objective_function_name.compare("griewank") == 0) {
            min_bound[i] = -600;
            max_bound[i] = 600;
        } else if (objective_function_name.compare("rastrigin") == 0) {
            min_bound[i] = -5.12;
            max_bound[i] = 5.12;
        } else if (objective_function_name.compare("rosenbrock") == 0) {
            min_bound[i] = -5;
            max_bound[i] = 10;
        }
    }

    MYSQL *conn = mysql_init(NULL);

    if (conn == NULL) {
        cerr << "Error initializing mysql: " << mysql_errno(conn) << ", '" << mysql_error(conn) << "'" << endl;
        exit(1);
    }

    string db_host, db_name, db_password, db_user;
    get_argument(arguments, "--db_host", true, db_host);
    get_argument(arguments, "--db_name", true, db_name);
    get_argument(arguments, "--db_user", true, db_user);
    get_argument(arguments, "--db_password", true, db_password);

    if (mysql_real_connect(conn, db_host.c_str(), db_user.c_str(), db_password.c_str(), db_name.c_str(), 0, NULL, 0) == NULL) {
        cerr << "Error connecting to database: " << mysql_errno(conn) << ", '" << mysql_error(conn) << "'" << endl;
        exit(1);
    }

    string search_type;
    get_argument(arguments, "--search_type", true, search_type);

    try {
        if (search_type.compare("anm") == 0 && argument_exists(arguments, "--create_tables"))    AsynchronousNewtonMethodDB::create_tables(conn);
        if (search_type.compare("de") == 0  && argument_exists(arguments, "--create_tables"))    DifferentialEvolutionDB::create_tables(conn);
        if (search_type.compare("ps") == 0  && argument_exists(arguments, "--create_tables"))    ParticleSwarmDB::create_tables(conn);
    } catch (string err_msg) {
        cout << "Creating tables for search '" << search_type << "' failed with message: " << endl;
        cout << "    " << err_msg << endl;
        exit(0);
    }

    string search_name;
    get_argument(arguments, "--search_name", true, search_name);

    try {
        if (search_type.compare("ps") == 0) {
            if (ParticleSwarmDB::search_exists(conn, search_name)) {
                cout << "Restarting database particle swarm search called '" << search_name << "'." << endl;
                ParticleSwarmDB ps(conn, search_name);
                ps.iterate(f);
            } else {
                cout << "Creating new database particle swarm search called '" << search_name << "'." << endl;
                ParticleSwarmDB ps(conn, min_bound, max_bound, arguments);
//                exit(1);
                ps.iterate(f);
            }

        } else if (search_type.compare("de") == 0) {
            if (DifferentialEvolutionDB::search_exists(conn, search_name)) {
                cout << "Restarting database differential evolution search called '" << search_name << "'." << endl;
                DifferentialEvolutionDB de(conn, search_name);
                de.iterate(f);
            } else {
                cout << "Creating new database differential evolution search called '" << search_name << "'." << endl;
                DifferentialEvolutionDB de(conn, min_bound, max_bound, arguments);
 //               exit(1);
                de.iterate(f);
            }

        } else if (search_type.compare("anm") == 0) {
            if (AsynchronousNewtonMethodDB::search_exists(conn, search_name)) {
                cout << "Restarting database asynchronous newton method search called '" << search_name << "'." << endl;
                AsynchronousNewtonMethodDB anm(conn, search_name);
                anm.iterate(f);
            } else {
                cout << "Creating new database asynchronous newton method search called '" << search_name << "'." << endl;
                AsynchronousNewtonMethodDB anm(conn, min_bound, max_bound, radius, arguments);
 //               exit(1);
                anm.iterate(f);
            }

        } else {
            cerr << "Improperly specified search type: '" << search_type.c_str() << "'" << endl;
            cerr << "Possibilities are:" << endl;
            cerr << "    anm    -       asynchronous newton method" << endl;
            cerr << "    de     -       differential evolution" << endl;
            cerr << "    ps     -       particle swarm optimization" << endl;
            exit(1);
        }
    } catch (string err_msg) {
        cout << "search failed with error message: " << endl;
        cout << "    " << err_msg << endl;
    }

    return 0;
}
