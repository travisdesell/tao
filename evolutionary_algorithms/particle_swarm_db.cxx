#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>

#include "particle_swarm_db.hxx"
#include "vector_io.hxx"
#include "arguments.hxx"

/**
 *  From MYSQL
 */
#include "mysql.h"

using namespace std;

/**
 *  The following construct a ParticleSwarm from a database entry
 */
ParticleSwarmDB::ParticleSwarmDB(MYSQL *conn, string name) throw (string) {
    ostringstream oss;
    oss << "SELECT * FROM particle_swarm WHERE id = " << id;
    string query = oss.str();
    cout << query << endl;

    construct_from_database(conn, query);
}

ParticleSwarmDB::ParticleSwarmDB(MYSQL *conn, int id) throw (string) {
    ostringstream oss;
    oss << "SELECT * FROM particle_swarm WHERE name = '" << name << "'";
    string query = oss.str();
    cout << query << endl;

    construct_from_database(conn, query);
}

void 
ParticleSwarmDB::construct_from_database(MYSQL *conn, string query) throw (string) {
    mysql_query(conn, query.c_str());

    MYSQL_RES *result = mysql_store_result(conn);

    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        construct_from_database(conn, row);
        mysql_free_result(result);

        cout << this << endl;
    } else {
        ostringstream ex_msg;
        ex_msg << "ERROR: could not get particle swarm from query: '" << query << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }
}

void 
ParticleSwarmDB::construct_from_database(MYSQL *conn, MYSQL_ROW row) throw (string) {
    id = atoi(row[0]);
    name = row[1];

    //Inherited from ParticleSwarm
    inertia = atof(row[2]);
    global_best_weight = atof(row[3]);
    local_best_weight = atof(row[4]);
    initial_velocity_scale = atof(row[5]);

    current_particle = atoi(row[6]);
    initialized_individuals = atoi(row[7]);

    string_to_vector_2d<double>(row[8], atof, particles);
    string_to_vector_2d<double>(row[9], atof, local_bests);
    string_to_vector_2d<double>(row[10], atof, velocities);
    string_to_vector<double>(row[11], atof, local_best_fitnesses);

    //calculate global_best and global_best_fitness
    global_best_fitness = -numeric_limits<double>::max();
    for (uint32_t i = 0; i < local_bests.size(); i++) {
        if (global_best_fitness < local_best_fitnesses[i]) {
            global_best.assign(local_bests[i].begin(), local_bests[i].end());
            global_best_fitness = local_best_fitnesses[i];
        }
    }

    //inherited from EvolutionaryAlgorithm
    current_iteration = atoi(row[12]);
    maximum_iterations = atoi(row[13]);
    individuals_created = atoi(row[14]);
    maximum_created = atoi(row[15]);
    individuals_reported = atoi(row[16]);
    maximum_reported = atoi(row[17]);

    population_size = atoi(row[18]);
    string_to_vector<double>(row[19], atof, min_bound);
    string_to_vector<double>(row[20], atof, max_bound);
    number_parameters = min_bound.size();
}


/**
 *  Insert a created particle swarm to a database.
 */
void
ParticleSwarmDB::insert_to_database(MYSQL *conn) throw (string) {
}

/**
 *  The following constructors create new ParticleSwarms and insert them into the database.
 */
//Create a particle swarm entirely from arguments
ParticleSwarmDB::ParticleSwarmDB(MYSQL *conn, const vector<string> &arguments) throw (string) : ParticleSwarm(arguments) {
    get_argument(arguments, "--search_name", true, name);
    insert_to_database(conn);
}


//Create a particle swarm from arguments and a given min and max bound
ParticleSwarmDB::ParticleSwarmDB( MYSQL *conn,
                                  const vector<double> &min_bound,            /* min bound is copied into the search */
                                  const vector<double> &max_bound,            /* max bound is copied into the search */
                                  const vector<string> &arguments
                                ) throw (string) : ParticleSwarm(min_bound, max_bound, arguments) {
    get_argument(arguments, "--search_name", true, name);
    insert_to_database(conn);
}

//Create a particle swarm entirely from defined parameters.
ParticleSwarmDB::ParticleSwarmDB( MYSQL *conn,
                                  const string name,
                                  const vector<double> &min_bound,            /* min bound is copied into the search */
                                  const vector<double> &max_bound,            /* max bound is copied into the search */
                                  const uint32_t population_size,
                                  const double inertia,                            /* intertia */
                                  const double global_best_weight,                 /* global best weight */
                                  const double local_best_weight,                  /* local best weight */
                                  const double initial_velocity_scale,             /* A scale for the initial velocities of particles so it doesn't immediately go to the bounds */
                                  const uint32_t maximum_iterations                /* default value is 0 which means no termination */
                                ) throw (string) : ParticleSwarm(min_bound, max_bound, population_size, inertia, global_best_weight, local_best_weight, initial_velocity_scale, maximum_iterations) {
    this->name = name;
    insert_to_database(conn);
}

ParticleSwarmDB::~ParticleSwarmDB() {
}

void
ParticleSwarmDB::new_individual(uint32_t &id, vector<double> &parameters) throw (string) {

}

void
ParticleSwarmDB::insert_individual(uint32_t id, const vector<double> &parameters, double fitness) throw (string) {
}

