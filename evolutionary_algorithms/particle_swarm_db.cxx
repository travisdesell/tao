#include <iostream>
#include <vector>

#include "db_particle_swarm.hpp"
#include "test_functions.hpp"

using namespace std;

int main(int argc, char** argv) {
    initialize_fgdo();

    string name = "test";
    cout << "MySQL client version: " <<  mysql_get_client_info() << endl;

    MYSQL *conn = mysql_init(NULL);

    mysql_real_connect(conn, "localhost", "boincadm", "b0infp@sswb", "milkyway", 0, NULL, 0);

    DBParticleSwarm *ps = new DBParticleSwarm(conn, string(argv[1]));

    mysql_close(conn);

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
