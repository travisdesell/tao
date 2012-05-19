#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>

#include "particle_swarm_db.hxx"
#include "vector_io.hxx"

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
    inertia = atod(row[2]);
    global_best_weight = atod(row[3]);
    local_best_weight = atod(row[4]);
    initial_velocity_scale = atod(row[5]);

    current_particle = atoi(row[6]);

    //particles
    //local_bests
    //velocities
    //local_best_fitnesses
    //can calculate global_best and global_best_fitnesses from these

    ostringstream oss;
    oss << "SELECT * FROM particle WHERE particle_swarm_id = " << this->id;
    mysql_query(conn, oss.str().c_str());
    MYSQL_RES *result = mysql_store_result(conn);

    if (result) {
        int num_results = mysql_num_rows(result);
        if (num_results > number_particles) {
            cout << "ERROR: got " << num_results << " results when looking up particles for search " << name << ", with a max number of particles: " << number_particles;
            return;
        }

        global_best = NULL;
        while ((row = mysql_fetch_row(result))) {
            try {
                DBParticle* p = new DBParticle(row);
                particles->at(p->get_position()) = p;

                cout << p << endl;

                if (global_best == NULL || p->get_fitness() > global_best->get_fitness()) global_best = p;
            } catch (string exception_message) {
                cerr << exception_message << endl;
            }
        }
        mysql_free_result(result);
    }

    //inherited from EvolutionaryAlgorithm
    current_iteration = atoi(row[7]);
    maximum_iteration = atoi(row[7]);
    created_individuals = atoi(row[7]);
    maximum_created = atoi(row[7]);
    reported_individuals = atoi(row[7]);
    maximum_reported = atoi(row[7]);

    population_size = atoi(row[8]);
    min_bound = string_to_vector<double>(row[8], atod);
    max_bound = string_to_vector<double>(row[8], atod);
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
void
ParticleSwarmDB::ParticleSwarmDB(MYSQL *conn, const vector<string> &arguments) throw (string) : ParticleSwarm(arguments) {
    get_argument(arguments, "--search_name", true, name);
    insert_to_database(conn);
}


//Create a particle swarm from arguments and a given min and max bound
void
ParticleSwarmDB::ParticleSwarmDB( MYSQL *conn,
                                  const vector<double> &min_bound,            /* min bound is copied into the search */
                                  const vector<double> &max_bound,            /* max bound is copied into the search */
                                  const vector<string> &arguments
                                ) throw (string) : ParticleSwarm(min_bound, max_bound, arguments) {
    get_argument(arguments, "--search_name", true, name);
    insert_to_database(conn);
}

//Create a particle swarm entirely from defined parameters.
void
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
    this->name(name);
    insert_to_database(conn);
}

~ParticleSwarmDB() {
}

void
ParticleSwarmDB::new_individual(uint32_t &id, vector<double> &parameters) throw (string) {

}

void
ParticleSwarmDB::insert_individual(uint32_t id, const vector<double> &parameters, double fitness) throw (string) {
}




DBParticle(int _particle_swarm_id, int _position, vector<double> *min_bound, vector<double> *max_bound) : Particle(_position, min_bound, max_bound) {
    particle_swarm_id = _particle_swarm_id;

    //Need to commit this to the database
    ostringstream oss;
    oss << "INSERT INTO particle "
        << "SET fitness = " << fitness
        << ", parameters = '" << vector_to_string<double>(parameters)
        << "', local_best = '" << vector_to_string<double>(local_best)
        << "', velocity = '" << vector_to_string<double>(velocity)
        << "', metadata = '" << metadata << "'"
        << ", particle_swarm_id = " << particle_swarm_id
        << ", position = " << position;
}

/**
 *  Create a particle from a database row.
 */
DBParticle(MYSQL_ROW row) {
    init(row);
}

/**
 *  Create a specific particle from the database.
 */
DBParticle(MYSQL *conn, int _particle_swarm_id, int _position) throw (string) {
    ostringstream oss;
    oss << "SELECT * FROM particle WHERE particle_swarm_id = " << particle_swarm_id << " AND position = " << position;
    cout << oss.str() << endl;
    mysql_query(conn, oss.str().c_str());

    MYSQL_RES *result = mysql_store_result(conn);

    cout << "creating a particle with particle swarm id " << particle_swarm_id << " and position " << position << endl;
    if (result) {
        init(mysql_fetch_row(result));
        mysql_free_result(result);
    } else {
        ostringstream ex_msg;
        ex_msg << "EXCEPTION: Could not find particle in database, particle_swarm_id: "
            << particle_swarm_id << ", position: " << position << ". Thrown on " 
            << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }
}

void init(MYSQL_ROW row) {
    particle_swarm_id = atoi(row[0]);
    position = atoi(row[1]);

    fitness = atof(row[2]);

    parameters = string_to_vector<double>(row[3], atof);
    velocity = string_to_vector<double>(row[4], atof);
    local_best = string_to_vector<double>(row[5], atof);

    metadata = row[6];

    //            cout << "Created Particle: " << to_string() << endl;
}

/**
 *  Update a particle in the database.
 */
void update_database(MYSQL *conn) {
    ostringstream oss;
    oss << "UPDATE particle "
        << "SET fitness = " << fitness 
        << ", parameters = '" << vector_to_string<double>(parameters) 
        << "', local_best = '" << vector_to_string<double>(local_best) 
        << "', velocity = '" << vector_to_string<double>(velocity) 
        << "', metadata = '" << metadata << "'"
        << "WHERE particle_swarm_id = " << particle_swarm_id << " AND position = " << position;

    cout << "UPDATING DATABASE: " << oss.str() << endl;
    //            mysql_query(conn, oss.str().c_str());
}

friend ostream& operator<< (ostream& stream, DBParticle* particle) {
    stream << "[db_particle: " << particle->position << ", fitness: " << particle->fitness << ", parameters: " << vector_to_string(particle->parameters) << ", velocity: " << vector_to_string(particle->velocity) << ", metadata: " << particle->metadata << "]";
    //            stream << "[particle: " << particle->position << ", fitness: " << particle->fitness << ", parameters: " << vector_to_string(particle->parameters) << ", velocity: " << vector_to_string(particle->velocity) << ", local_best: " << vector_to_string(particle->local_best) << ", metadata: " << particle->metadata << "]";
    return stream;
}

friend ostream& operator<< (ostream& stream, DBParticleSwarm* ps) {
    stream << "[db_particle_swarm: " << endl
        << "  id: " << ps->id << endl
        << "  app_id: " << ps->app_id << endl
        << "  name: " << ps->name << endl
        << "  evaluations_done: " << ps->evaluations_done << endl
        << "  particles_generated: " << ps->particles_generated << endl 
        << "  max_evaluations: " << ps->maximum_evaluations << endl
        << "  requires_seeding: " << ps->requires_seeding << endl
        << "  w: " << ps->w << endl
        << "  c1: " << ps->c1 << endl
        << "  c2: " << ps->c2 << endl
        << "  number_particles: " << ps->number_particles << endl
        << "  min_bound: " << vector_to_string(ps->min_bound) << endl
        << "  max_bound: " << vector_to_string(ps->max_bound) << endl
        << "  command_line_options: " << ps->command_line_options << endl
        << "  workunit_xml_filename: " << ps->workunit_xml_filename << endl
        << "  result_xml_filename: " << ps->result_xml_filename << endl
        << "  input_filenames: " << vector_to_string(ps->input_filenames) << endl
        << "  extra_xml: " << ps->extra_xml << endl
        << "]";
    return stream;
}


int main(int argc, char** argv) {
    check_stop_daemons();

    DB_APP app;

    retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
                "Can't parse config.xml: %s\n", boincerror(retval)
                );  
        exit(1);
    }   

    log_messages.printf(MSG_NORMAL, "Starting\n");

    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "Can't open DB\n");
        exit(1);
    }
    sprintf(buf, "where name='%s'", app.name);
    retval = app.lookup(buf);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "Can't find app\n");
        exit(1);
    }


    /*
    for (int i = 0; i < max_evaluations; i++) {
        vector<Particle*> *generated = ps->generate_particles(number_to_generate);
        for (int j = 0; j < number_to_generate; j++) {
//            generated->at(j)->set_fitness( ackley( generated->at(j)->get_parameters() ) );
//            generated->at(j)->set_fitness( griewank( generated->at(j)->get_parameters() ) );
//            generated->at(j)->set_fitness( rastrigin( generated->at(j)->get_parameters() ) );
//            generated->at(j)->set_fitness( rosenbrock( generated->at(j)->get_parameters() ) );
            generated->at(j)->set_fitness( sphere( generated->at(j)->get_parameters() ) );
            ps->insert_particle(generated->at(j));

            delete generated->at(j);
        }
        delete generated;
    }
    */
    delete ps;
}
