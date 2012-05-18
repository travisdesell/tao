#include <iostream>
#include <vector>

#include "particle_swarm_db.hxx"

/**
 *  From BOINC
 */

#include "config.h"
#include "boinc_db.h"
#include "parse.h"
#include "util.h"
#include "error_numbers.h"
#include "str_util.h"
#include "svn_version.h"
#include "sched_msgs.h"

/*
 * End BOINC includes
 */

using namespace std;

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
