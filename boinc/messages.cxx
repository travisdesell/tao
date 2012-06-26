#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include "mysql.h"

//From TAO
#include "evolutionary_algorithms/particle_swarm_db.hxx"
#include "evolutionary_algorithms/differential_evolution_db.hxx"

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

    EvolutionaryAlgorithmDB *eadb;
    while (searches.size() > 0) {
        eadb = searches.back();

        cout << "\t" << eadb->get_name() << ", " << eadb->get_id() << endl;
        searches.pop_back();
        delete eadb;
    }
}
