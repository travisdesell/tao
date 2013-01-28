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

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include <boost/filesystem.hpp>

#include "mysql.h"

//From BOINC
#include "boinc_db.h"
#include "error_numbers.h"
#include "backend_lib.h"
#include "parse.h"
#include "util.h"
#include "svn_version.h"

#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "str_util.h"

//From TAO
#include "messages.hxx"
#include "asynchronous_algorithms/particle_swarm_db.hxx"
#include "asynchronous_algorithms/differential_evolution_db.hxx"
#include "util/statistics.hxx"

#include "undvc_common/arguments.hxx"
#include "undvc_common/vector_io.hxx"
#include "undvc_common/file_io.hxx"

DB_APP app;

using namespace std;

int main(int argc, char **argv) {
    vector<string> arguments(argv, argv + argc);

    //need to initialize the BOINC database
    int retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "Can't parse config.xml: %s\n", boincerror(retval));
        exit(1);
    }

    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't open db\n");
        exit(1);
    }

    string app_name;
    bool app_found = get_argument(arguments, "--app", false, app_name);
    if (!app_found) {
        //app argument not found, print out apps in the database
        cout << "Need to specify which application with the '--app <name>' command line argument." << endl;
        print_applications(boinc_db.mysql);
        exit(1);
    }

    DB_APP app;
    char buf[256];
    sprintf(buf, "where name='%s'", app_name.c_str());
    if (app.lookup(buf)) {
        log_messages.printf(MSG_CRITICAL, "can't find app %s\n", app_name.c_str());
        print_applications(boinc_db.mysql);
        exit(1);
    }

    try {
        string search_name;
        string search_type;
        string individual_type;

        bool search_found = get_argument(arguments, "--search_name", false, search_name);
        if (!search_found) {

            cout << "Need to specify which search with the '--search_name <name>' command line argument." << endl;
            print_searches(boinc_db.mysql, app.id);

        } else {

            try {
                vector<Individual> individuals;
                if (search_name.substr(0,3).compare("ps_") == 0) {
                    ParticleSwarmDB ps(boinc_db.mysql, search_name);
                    cout << ps << endl;

                    ps.get_individuals(individuals);

                } else if (search_name.substr(0,3).compare("de_") == 0) {
                    DifferentialEvolutionDB de(boinc_db.mysql, search_name);
                    cout << de << endl;
                    de.get_individuals(individuals);

                } else {
                    cerr << "An an unkown search type was specified (searches need to start with de_ or ps_): '" << search_name << "'" << endl;
                }   

                uint32_t print_best_x = 0;
                get_argument(arguments, "--print_best", false, print_best_x);
                if (individuals.size() > 0 && print_best_x > 0) {
                    sort(individuals.begin(), individuals.end());

                    double best, average, median, worst;
                    calculate_fitness_statistics(individuals, best, average, median, worst);

                    cout << "best fitness: " << best << endl;
                    cout << "median fitness: " << median << endl;
                    cout << "average fitness: " << average << endl;
                    cout << "worst fitness: " << worst << endl;
                    cout << endl;

                    uint32_t start = individuals.size() - (print_best_x + 1);
                    if (start < 0) start = 0;
                    cout << "The best " << individuals.size() - start << " of " << individuals.size() << " individuals are (position, fitness, parameters, metadata):" << endl;

                    for (uint32_t i = individuals.size() - (print_best_x + 1); i < individuals.size(); i++) {
                        cout << "\t" << individuals[i] << endl;
                    }
                } else {
                    cout << "You can print the best X individuals by using the '--print_best <int>' command line argument." << endl;
                }

            } catch (string err_msg) {
                cerr << "Could not find search: '" << search_name << "' threw error: " << err_msg << endl;
            }   
        }
    } catch (string err_msg) {
        cerr << "Error getting the search from the database." << endl;
        cerr << "threw message: '" << err_msg << "'" << endl;
    }
}
