#ifndef FGDO_DB_PARTICLE_SWARM
#define FGDO_DB_PARTICLE_SWARM

#include <iostream>
#include <sstream>
#include <vector>

//#include <my_global.h>
#include <mysql.h>

#include "particle_swarm.hpp"
#include "../util/util.hpp"

using namespace std;

class DBParticle : public Particle {
    protected:
        int particle_swarm_id;

    public:
        /**
         *  Create a new particle, and insert it into the database.
         */
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
};

class DBParticleSwarm : public ParticleSwarm {
    private:
        unsigned int id;
        unsigned int app_id;
        int requires_seeding;

        string command_line_options;
        string workunit_xml_filename;
        string result_xml_filename;

        vector<string>* input_filenames;

        string extra_xml;

    public:

        /**
         *  Read a particle swarm from the database with a given id.
         */
        DBParticleSwarm(MYSQL *conn, int id) throw (string) {
            ostringstream oss;
            oss << "SELECT * FROM particle_swarm WHERE id = " << id;
            string query = oss.str();
            cout << query << endl;

            init_db(conn, query);
        }

        /**
         *  Read a particle swarm from the database with a given name.
         */
        DBParticleSwarm(MYSQL *conn, string name) throw (string) {
            ostringstream oss;
            oss << "SELECT * FROM particle_swarm WHERE name = '" << name << "'";
            string query = oss.str();
            cout << query << endl;

            init_db(conn, query);
        }

        ~DBParticleSwarm() {
            delete input_filenames;
        }

        void init_db(MYSQL *conn, string query) throw (string) {
            mysql_query(conn, query.c_str());

            MYSQL_RES *result = mysql_store_result(conn);

            if (result) {
                MYSQL_ROW row = mysql_fetch_row(result);
                init_db(conn, row);
                mysql_free_result(result);

                cout << this << endl;
            } else {
                ostringstream ex_msg;
                ex_msg << "ERROR: could not get particle swarm from query: '" << query << "'. Thrown on " 
                       << __FILE__ << ":" << __LINE__;
                throw ex_msg.str();
            }
        }

        void init_db(MYSQL *conn, MYSQL_ROW row) {
            id = atoi(row[0]);
            app_id = atoi(row[1]);
            name = row[2];
            evaluations_done = atoi(row[3]);
            particles_generated = atoi(row[4]);
            maximum_evaluations = atoi(row[5]);
            requires_seeding = atoi(row[6]);

            w = atof(row[7]);
            c1 = atof(row[8]);
            c2 = atof(row[9]);

            number_particles = atoi(row[10]);
            particles = new vector<Particle*>;
            particles->resize(number_particles);

            min_bound = string_to_vector<double>(row[11], atof);
            max_bound = string_to_vector<double>(row[12], atof);

            command_line_options = row[13];
            workunit_xml_filename = row[14];
            result_xml_filename = row[15];

            input_filenames = string_to_vector<string>(row[16], convert_string);

            extra_xml = row[17];

//            cout << "Created a particle swarm: " << name << endl;

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

};

#endif
