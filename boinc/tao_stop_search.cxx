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
        exit(1);
    }

    try {
        string search_name;
        string search_type;
        string individual_type;

        bool search_found = get_argument(arguments, "--search_name", false, search_name);
        if (!search_found) {
            cout << "Need to specify which search with the '--search_name <name>' command line argument." << endl;
            cout << "Searches for app id " << app.id << " (name, id): " << endl;
            print_searches(boinc_db.mysql, app.id);
            exit(1);
        }

        int search_id;
        if (search_name.substr(0,3).compare("ps_") == 0) {
            search_type = "particle_swarm";
            individual_type = "particle";

            ParticleSwarmDB ps(boinc_db.mysql, search_name);
            search_id = ps.get_id();

        } else if (search_name.substr(0,3).compare("de_") == 0) {
            search_type = "differential_evolution";
            individual_type = "de_individual";

            DifferentialEvolutionDB de(boinc_db.mysql, search_name);
            search_id = de.get_id();

        } else {
            cerr << "Improperly specified search name: '" << search_name <<"'" << endl;
            cerr << "Search name must begin with either:" << endl;
            cerr << "    'de_'     -       for differential evolution" << endl;
            cerr << "    'ps_'     -       for particle swarm optimization" << endl;
            exit(1);
        }

        ostringstream query, query2, query3;
        if (argument_exists(arguments, "--delete")) {
            query << "DELETE FROM " << search_type
                  << " WHERE name = '" << search_name << "'";

            query2 << "DELETE FROM " << individual_type
                   << " WHERE " << search_type << "_id = " << search_id;

            query3 << "DELETE FROM tao_workunit_information WHERE "
                   << "search_name = '" << search_name << "' AND app_id = " << app.id;
        } else {
            query << "UPDATE " << search_type
                  << " SET maximum_created = 1"
                  << " WHERE name = '" << search_name << "'";
        }

        mysql_query(boinc_db.mysql, query.str().c_str());

        if (mysql_errno(boinc_db.mysql) != 0) {
            ostringstream ex_msg;
            ex_msg << "ERROR: updating database with query: '" << query.str() << "'. Error: " << mysql_errno(boinc_db.mysql) << " -- '" << mysql_error(boinc_db.mysql) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
            throw ex_msg.str();
        }

        if (argument_exists(arguments, "--delete")) {
            mysql_query(boinc_db.mysql, query2.str().c_str());

            if (mysql_errno(boinc_db.mysql) != 0) {
                ostringstream ex_msg;
                ex_msg << "ERROR: updating database with query: '" << query2.str() << "'. Error: " << mysql_errno(boinc_db.mysql) << " -- '" << mysql_error(boinc_db.mysql) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
                throw ex_msg.str();
            }

            mysql_query(boinc_db.mysql, query3.str().c_str());

            if (mysql_errno(boinc_db.mysql) != 0) {
                ostringstream ex_msg;
                ex_msg << "ERROR: updating database with query: '" << query3.str() << "'. Error: " << mysql_errno(boinc_db.mysql) << " -- '" << mysql_error(boinc_db.mysql) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
                throw ex_msg.str();
            }

            cout << "Successfully deleted search with queries: " << endl;
            cout << "\t" << query.str() << "'." << endl;
            cout << "\t" << query2.str() << "'." << endl;
            cout << "\t" << query3.str() << "'." << endl;
        }

    } catch (string err_msg) {
            cerr << "Error putting the search into the database." << endl;
            cerr << "threw message: '" << err_msg << "'" << endl;
    }

    cout << "Search successfully stopped." << endl;
}
