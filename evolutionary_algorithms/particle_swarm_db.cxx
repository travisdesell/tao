#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <limits>

#include "particle_swarm_db.hxx"
#include "vector_io.hxx"
#include "arguments.hxx"

/**
 *  From MYSQL
 */
#include "mysql.h"

using namespace std;

void
ParticleSwarmDB::check_name(string name) throw (string) {
    if (!name.substr(0,3).compare("ps_") == 0) {
        ostringstream err_msg;
        err_msg << "Improper name for ParticleSwarmDB '" << name << "' must start with 'ps_'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw err_msg.str();
    }   
}


/**
 *  The following construct a ParticleSwarm from a database entry
 */
ParticleSwarmDB::ParticleSwarmDB(MYSQL *conn, string name) throw (string) {
    check_name(name);
    this->conn = conn;

    ostringstream oss;
    oss << "SELECT * FROM particle_swarm WHERE name = '" << name << "'";
    cout << oss.str() << endl;

    construct_from_database(oss.str());
}

ParticleSwarmDB::ParticleSwarmDB(MYSQL *conn, int id) throw (string) {
    this->conn = conn;

    ostringstream oss;
    oss << "SELECT * FROM particle_swarm WHERE id = " << id;
    cout << oss.str() << endl;

    construct_from_database(oss.str());
}

bool
ParticleSwarmDB::search_exists(MYSQL *conn, std::string search_name) throw (std::string) {
    ostringstream query;
    query << "SELECT id FROM particle_swarm where name = '" << search_name << "'";

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
ParticleSwarmDB::create_tables(MYSQL *conn) throw (string) {
    ostringstream swarm_query;
    swarm_query << "CREATE TABLE `particle_swarm` ("
                << "    `id` int(11) NOT NULL AUTO_INCREMENT,"
                << "    `name` varchar(254) NOT NULL DEFAULT '',"
                << "    `inertia` double NOT NULL DEFAULT '1',"
                << "    `global_best_weight` double NOT NULL DEFAULT '2',"
                << "    `local_best_weight` double NOT NULL DEFAULT '2',"
                << "    `initial_velocity_scale` double NOT NULL DEFAULT '2',"
                << "    `current_particle` int(11) NOT NULL DEFAULT '0',"
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
                << "PRIMARY KEY (`id`),"
                << "UNIQUE KEY `name` (`name`)"
                << ") ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=latin1";

    cout << "creating particle_swarm table with: " << endl << swarm_query.str() << endl << endl;

    if (mysql_query(conn, swarm_query.str().c_str())) {
        ostringstream ex_msg;
        ex_msg << "ERROR: could not create particle swarm table with query: '" << swarm_query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }

    ostringstream particle_query;
    particle_query  << "CREATE TABLE `particle` ("
                    << "    `particle_swarm_id` int(11) NOT NULL,"
                    << "    `position` int(11) NOT NULL,"
                    << "    `local_best_fitness` double NOT NULL,"
                    << "    `parameters` varchar(2048) NOT NULL,"
                    << "    `velocity` varchar(2048) NOT NULL,"
                    << "    `local_best` varchar(2048) NOT NULL,"
                    << "    `seed` int(32) UNSIGNED,"
                    << "PRIMARY KEY (`particle_swarm_id`,`position`)"
                    << ") ENGINE=InnoDB DEFAULT CHARSET=latin1";

    cout << "creating particle table with: " << endl << particle_query.str() << endl << endl;

    if (mysql_query(conn, particle_query.str().c_str())) {
        ostringstream ex_msg;
        ex_msg << "ERROR: could not create particle table with query: '" << particle_query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }
}

void 
ParticleSwarmDB::construct_from_database(string query) throw (string) {
    mysql_query(conn, query.c_str());

    MYSQL_RES *result = mysql_store_result(conn);

    if (result != NULL) {
        MYSQL_ROW row = mysql_fetch_row(result);
        
        if (row == NULL) {
            ostringstream ex_msg;
            ex_msg << "ERROR: could not construct particle swarm '" << name << "' from database, it does not exist. Thrown on " << __FILE__ << ":" << __LINE__;
            throw ex_msg.str();
        }

        construct_from_database(row);
        mysql_free_result(result);
    } else {
        ostringstream ex_msg;
        ex_msg << "ERROR: could not get particle swarm from query: '" << query << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }
}


void 
ParticleSwarmDB::construct_from_database(MYSQL_ROW row) throw (string) {
    id = atoi(row[0]);
    name = row[1];

    //Inherited from ParticleSwarm
    inertia = atof(row[2]);
    global_best_weight = atof(row[3]);
    local_best_weight = atof(row[4]);
    initial_velocity_scale = atof(row[5]);

    current_particle = atoi(row[6]);
    initialized_individuals = atoi(row[7]);

    //inherited from EvolutionaryAlgorithm
    current_iteration = atoi(row[8]);
    maximum_iterations = atoi(row[9]);
    individuals_created = atoi(row[10]);
    maximum_created = atoi(row[11]);
    individuals_reported = atoi(row[12]);
    maximum_reported = atoi(row[13]);

    population_size = atoi(row[14]);
    string_to_vector<double>(row[15], atof, min_bound);
    string_to_vector<double>(row[16], atof, max_bound);
    number_parameters = min_bound.size();

    //Get the particle information from the database
    ostringstream oss;
    oss << "SELECT * FROM particle WHERE particle_swarm_id = " << this->id << " ORDER BY position";
    mysql_query(conn, oss.str().c_str());
    MYSQL_RES *result = mysql_store_result(conn);

    cout << oss.str() << endl;

    local_best_fitnesses.resize(population_size, -numeric_limits<double>::max());
    local_bests.resize(population_size, vector<double>(number_parameters, 0.0));
    particles.resize(population_size, vector<double>(number_parameters, 0.0));
    velocities.resize(population_size, vector<double>(number_parameters, 0.0));
    seeds.resize(population_size, 0);
    EvolutionaryAlgorithm::initialize();    //to initialize the random number generator

    if (result != NULL) {
        uint32_t num_results = mysql_num_rows(result);
        if (num_results != population_size) {
            ostringstream ex_msg;
            ex_msg << "ERROR: got " << num_results << " results when looking up particles for search " << name << ", with a population size: " << population_size << ". Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
            throw ex_msg.str();
        }   

        MYSQL_ROW particle_row;

        while ((particle_row = mysql_fetch_row(result))) {
            int particle_id = atoi(particle_row[1]);
            local_best_fitnesses[particle_id] = atof(particle_row[2]);

            if (local_best_fitnesses[particle_id] < -1.79768e+308) {
                local_best_fitnesses[particle_id] = -numeric_limits<double>::max();
            }   

            string_to_vector<double>(particle_row[3], atof, particles[particle_id]);
            string_to_vector<double>(particle_row[4], atof, velocities[particle_id]);
            string_to_vector<double>(particle_row[5], atof, local_bests[particle_id]);
            seeds[particle_id] = atoi(particle_row[6]);
         }   
        mysql_free_result(result);
    } else {
        ostringstream ex_msg;
        ex_msg << "ERROR: looking up particles with query: '" << oss.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }

    //calculate global_best and global_best_fitness
    global_best_fitness = -numeric_limits<double>::max();
    for (uint32_t i = 0; i < local_bests.size(); i++) {
        if (global_best_fitness < local_best_fitnesses[i]) {
            global_best.assign(local_bests[i].begin(), local_bests[i].end());
            global_best_fitness = local_best_fitnesses[i];
        }
    }
}


/**
 *  Insert a created particle swarm to a database.
 */
void
ParticleSwarmDB::insert_to_database() throw (string) {
    ostringstream query;

    query << "INSERT INTO particle_swarm"
          << " SET "
          << "  name = '" << name << "'"
          << ", inertia = " << inertia
          << ", global_best_weight = " << global_best_weight
          << ", local_best_weight = " << local_best_weight
          << ", initial_velocity_scale = " << initial_velocity_scale
          << ", current_particle = " << current_particle
          << ", initialized_individuals = " << initialized_individuals
          << ", current_iteration = " << current_iteration  
          << ", maximum_iterations = " << maximum_iterations
          << ", individuals_created = " << individuals_created
          << ", maximum_created = " << maximum_created  
          << ", individuals_reported = " << individuals_reported
          << ", maximum_reported = " << maximum_reported 
          << ", population_size = " << population_size
          << ", min_bound = '" << vector_to_string<double>(min_bound) << "'"
          << ", max_bound = '" << vector_to_string<double>(max_bound) << "'";

    mysql_query(conn, query.str().c_str());

    MYSQL_RES *result;
    if ((result = mysql_store_result(conn)) == 0 && mysql_field_count(conn) == 0 && mysql_insert_id(conn) != 0) {
        id = mysql_insert_id(conn);
    } else {
        ostringstream ex_msg;
        ex_msg << "ERROR: could not get particle swarm id from insert query: '" << query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }
    mysql_free_result(result);

    for (uint32_t i = 0; i < population_size; i++) {
        ostringstream particle_query;
        particle_query << "INSERT INTO particle"
                       << " SET "
                       << "  particle_swarm_id = " << id
                       << ", position = " << i
                       << ", local_best_fitness = " << local_best_fitnesses[i]
                       << ", parameters = '" << vector_to_string<double>(particles[i]) << "'"
                       << ", velocity = '" << vector_to_string<double>(velocities[i]) << "'"
                       << ", local_best = '" << vector_to_string<double>(local_bests[i]) << "'"
                       << ", seed = " << seeds[i];

        mysql_query(conn, particle_query.str().c_str());
//        result = mysql_store_result(conn);

        if (mysql_errno(conn) != 0) {
            ostringstream ex_msg;
            ex_msg << "ERROR: creating particle with insert query: '" << particle_query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
            throw ex_msg.str();
        }
        mysql_free_result(result);
    }
}

/**
 *  The following constructors create new ParticleSwarms and insert them into the database.
 */
//Create a particle swarm entirely from arguments
ParticleSwarmDB::ParticleSwarmDB(MYSQL *conn, const vector<string> &arguments) throw (string) : ParticleSwarm(arguments) {
    this->conn = conn;
    get_argument(arguments, "--search_name", true, name);
    insert_to_database();
}


//Create a particle swarm from arguments and a given min and max bound
ParticleSwarmDB::ParticleSwarmDB( MYSQL *conn,
                                  const vector<double> &min_bound,            /* min bound is copied into the search */
                                  const vector<double> &max_bound,            /* max bound is copied into the search */
                                  const vector<string> &arguments
                                ) throw (string) : ParticleSwarm(min_bound, max_bound, arguments) {
    this->conn = conn;
    get_argument(arguments, "--search_name", true, name);
    check_name(name);
    insert_to_database();
}

//Create a particle swarm entirely from defined parameters.
ParticleSwarmDB::ParticleSwarmDB( MYSQL *conn,
                                  const string name,
                                  const vector<double> &min_bound,                 /* min bound is copied into the search */
                                  const vector<double> &max_bound,                 /* max bound is copied into the search */
                                  const uint32_t population_size,
                                  const double inertia,                            /* intertia */
                                  const double global_best_weight,                 /* global best weight */
                                  const double local_best_weight,                  /* local best weight */
                                  const double initial_velocity_scale,             /* A scale for the initial velocities of particles so it doesn't immediately go to the bounds */
                                  const uint32_t maximum_iterations                /* default value is 0 which means no termination */
                                ) throw (string) : ParticleSwarm(min_bound, max_bound, population_size, inertia, global_best_weight, local_best_weight, initial_velocity_scale, maximum_iterations) {
    this->conn = conn;
    this->name = name;
    check_name(name);
    insert_to_database();
}

//Create a particle swarm entirely from defined parameters.
ParticleSwarmDB::ParticleSwarmDB( MYSQL *conn,
                                  const string name,
                                  const vector<double> &min_bound,                 /* min bound is copied into the search */
                                  const vector<double> &max_bound,                 /* max bound is copied into the search */
                                  const uint32_t population_size,
                                  const double inertia,                            /* intertia */
                                  const double global_best_weight,                 /* global best weight */
                                  const double local_best_weight,                  /* local best weight */
                                  const double initial_velocity_scale,             /* A scale for the initial velocities of particles so it doesn't immediately go to the bounds */
                                  const uint32_t maximum_created,                  /* default value is 0 which means no termination */
                                  const uint32_t maximum_reported                  /* default value is 0 which means no termination */
                                ) throw (string) : ParticleSwarm(min_bound, max_bound, population_size, inertia, global_best_weight, local_best_weight, initial_velocity_scale, maximum_created, maximum_reported) {
    this->conn = conn;
    this->name = name;
    check_name(name);
    insert_to_database();
}


ParticleSwarmDB::~ParticleSwarmDB() {
}

void
ParticleSwarmDB::new_individual(uint32_t &id, vector<double> &parameters, uint32_t &seed) throw (string) {
    ParticleSwarm::new_individual(id, parameters, seed);
}

void
ParticleSwarmDB::new_individual(uint32_t &id, vector<double> &parameters) throw (string) {
    ParticleSwarm::new_individual(id, parameters);
}

bool
ParticleSwarmDB::insert_individual(uint32_t id, const vector<double> &parameters, double fitness, uint32_t seed) throw (string) {
    bool modified = ParticleSwarm::insert_individual(id, parameters, fitness);

    if (modified) {
//        MYSQL_RES *result;
        ostringstream particle_query;
        particle_query << "UPDATE particle"
                       << " SET "
                       << "  local_best_fitness = " << local_best_fitnesses[id]
                       << ", parameters = '" << vector_to_string<double>(particles[id]) << "'"
                       << ", velocity = '" << vector_to_string<double>(velocities[id]) << "'"
                       << ", local_best = '" << vector_to_string<double>(local_bests[id]) << "'"
                       << ", seed = " << seeds[id]
                       << " WHERE "
                       << "     particle_swarm_id = " << this->id
                       << " AND position = " << id;

        mysql_query(conn, particle_query.str().c_str());
//        result = mysql_store_result(conn);

        if (mysql_errno(conn) != 0) {
            ostringstream ex_msg;
            ex_msg << "ERROR: updating particle with query: '" << particle_query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
            throw ex_msg.str();
        }
//        mysql_free_result(result);

        ostringstream swarm_query;
        swarm_query << " UPDATE particle_swarm"
                    << " SET "
                    << "  current_particle = " << current_particle
                    << ", initialized_individuals = " << initialized_individuals
                    << ", current_iteration = " << current_iteration
                    << ", maximum_iterations = " << maximum_iterations
                    << ", individuals_created = " << individuals_created
                    << ", maximum_created = " << maximum_created
                    << ", individuals_reported = " << individuals_reported
                    << ", maximum_reported = " << maximum_reported
                    << " WHERE "
                    << "    id = " << this->id << endl;

//        cout << swarm_query.str() << endl;

        mysql_query(conn, swarm_query.str().c_str());
//        result = mysql_store_result(conn);

        if (mysql_errno(conn) != 0) {
            ostringstream ex_msg;
            ex_msg << "ERROR: updating particle_swarm with query: '" << particle_query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
            throw ex_msg.str();
        }
//        mysql_free_result(result);
    }

    return modified;
}

void
ParticleSwarmDB::print_to(ostream& stream) {
    stream  << "[ParticleSwarmDB " << endl
            << "    id = " << id << endl
            << "    name = '" << name << "'" << endl
            << "    inertia = " << inertia << endl
            << "    global_best_weight = " << global_best_weight << endl
            << "    local_best_weight = " << local_best_weight << endl
            << "    initial_velocity_scale = " << initial_velocity_scale << endl
            << "    current_particle = " << current_particle << endl
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
            << "]" << endl;

    for (uint32_t i = 0; i < population_size; i++) {
        stream << "    [Particle" << endl
               << "        particle_swarm_id = " << id << endl
               << "        position = " << i << endl
               << "        local_best_fitness = " << local_best_fitnesses[i] << endl
               << "        parameters = '" << vector_to_string<double>(particles[i]) << "'" << endl
               << "        velocity = '" << vector_to_string<double>(velocities[i]) << "'" << endl
               << "        local_best = '" << vector_to_string<double>(local_bests[i]) << "'" << endl
               << "        seed = " << seeds[i] << endl
               << "    ]" << endl;
    }
}

ostream& operator<< (ostream& stream, ParticleSwarmDB &ps) {
    ps.print_to(stream);
    return stream;
}
