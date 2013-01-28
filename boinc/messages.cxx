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

#include "mysql.h"

//From TAO
#include "asynchronous_algorithms/particle_swarm_db.hxx"
#include "asynchronous_algorithms/differential_evolution_db.hxx"

#include "messages.hxx"

using namespace std;

void get_applications(MYSQL *conn, vector<string> &names, vector<int> &ids) {
    string query = "SELECT name, id FROM app";

    mysql_query(conn, query.c_str());
    MYSQL_RES *result = mysql_store_result(conn);

    if (mysql_errno(conn) != 0) {
        cerr << "ERROR: getting applications with query: '" << query << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        exit(1);
    }

    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result))) {
        if (mysql_errno(conn) != 0) {
            cerr << "ERROR: getting row for applications with query: '" << query << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
            exit(1);
        }

        names.push_back(row[0]);
        ids.push_back(atoi(row[1]));
    }

    mysql_free_result(result);
}

void print_applications(MYSQL *conn) {
    vector<string> names;
    vector<int> ids;

    get_applications(conn, names, ids);

    cout << "Applications in the database (name, id):" << endl;
    for (uint32_t i = 0; i < names.size(); i++) {
        cout << "\t" << names[i] << ", " << ids[i] << endl;
    }
}

void print_searches(MYSQL *conn, int app_id) {
    vector<EvolutionaryAlgorithmDB*> searches;
    ParticleSwarmDB::add_searches(conn, app_id, searches);
    DifferentialEvolutionDB::add_searches(conn, app_id, searches);

    cout << "Searches for app id " << app_id << " (name, id, status): " << endl;
    EvolutionaryAlgorithmDB *eadb;
    while (searches.size() > 0) {
        eadb = searches.back();

        cout << "\t" << eadb->get_name() << ", " << eadb->get_id() << ", " << ((dynamic_cast<EvolutionaryAlgorithm*>(eadb))->is_running() ? "running" : "finished") << endl;
        searches.pop_back();
        delete eadb;
    }
}
