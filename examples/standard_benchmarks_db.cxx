#include <cmath>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <cfloat>
#include <string>
#include <stdint.h>

#include "particle_swarm.hxx"
#include "particle_swarm_db.hxx"
#include "differential_evolution.hxx"

//from undvc_common
#include "arguments.hxx"
#include "benchmarks.hxx"

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

int main(uint32_t argc /* number of command line arguments */, char **argv /* command line argumens */ ) {
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
        fprintf(stderr, "Improperly specified objective function: '%s'\n", objective_function_name.c_str());
        fprintf(stderr, "Possibilities are:\n");
        fprintf(stderr, "    sphere\n");
        fprintf(stderr, "    ackley\n");
        fprintf(stderr, "    griewank\n");
        fprintf(stderr, "    rastrigin\n");
        fprintf(stderr, "    rosenbrock\n");
        exit(0);
    }

    /**
     *  Initialize the arrays for the minimum and maximum bounds of the search space.
     *  These are different for the different objective functions.
     */
    uint32_t number_of_parameters;
    get_argument(arguments, "--n_parameters", true, number_of_parameters);
    vector<double> min_bound(number_of_parameters, 0);
    vector<double> max_bound(number_of_parameters, 0);

    for (uint32_t i = 0; i < number_of_parameters; i++) {        //arrays go from 0 to size - 1 (not 1 to size)
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
            min_bound[i] = 10;
        }
    }

    MYSQL *conn = mysql_init(NULL);

    if (conn == NULL) {
        printf("Error initializing mysql %u: %s\n", mysql_errno(conn), mysql_error(conn));
        exit(1);
    }

    string db_host, db_name, db_password, db_user;
    get_argument(arguments, "--db_host", true, db_host);
    get_argument(arguments, "--db_name", true, db_name);
    get_argument(arguments, "--db_user", true, db_user);
    get_argument(arguments, "--db_password", true, db_password);

    if (mysql_real_connect(conn, db_host.c_str(), db_user.c_str(), db_password.c_str(), db_name.c_str(), 0, NULL, 0) == NULL) {
        printf("Error connecting to database %u: %s\n", mysql_errno(conn), mysql_error(conn));
        exit(1);
    }

    string search_type;
    get_argument(arguments, "--search_type", true, search_type);

    string search_name;
    get_argument(arguments, "--search_name", true, search_name);

    try {
        if (search_type.compare("ps") == 0) {
//            ParticleSwarmDB::create_tables(conn);

            if (ParticleSwarmDB::search_exists(conn, search_name)) {
                cout << "Restarting database particle swarm search called '" << search_name << "'." << endl;
                ParticleSwarmDB ps(conn, search_name);
                ps.iterate(f);
            } else {
                cout << "Creating new database particle swarm search called '" << search_name << "'." << endl;
                ParticleSwarmDB ps(conn, min_bound, max_bound, arguments);
                ps.iterate(f);
            }

        } else if (search_type.compare("de") == 0) {
            DifferentialEvolutionDB::create_tables(conn);

            if (DifferentialEvolutionDB::search_exists(conn, search_name)) {
                cout << "Restarting database differential evolution search called '" << search_name << "'." << endl;
                DifferentialEvolutiondB de(conn, search_name);
                de.iterate(f);
            } else {
                cout << "Creating new database differential evolution search called '" << search_name << "'." << endl;
                DifferentialEvolutiondB de(conn, min_bound, max_bound, arguments);
                de.iterate(f);
            }

        } else {
            fprintf(stderr, "Improperly specified search type: '%s'\n", search_type.c_str());
            fprintf(stderr, "Possibilities are:\n");
            fprintf(stderr, "    de     -       differential evolution\n");
            fprintf(stderr, "    ps     -       particle swarm optimization\n");
            exit(0);
        }
    } catch (string err_msg) {
        cout << "search failed with error message: " << endl;
        cout << "    " << err_msg << endl;
    }

    return 0;
}
