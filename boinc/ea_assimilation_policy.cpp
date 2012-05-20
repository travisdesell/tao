#include <vector>
#include <string>
#include <cstdlib>

#include "boinc_db.h"
#include "error_numbers.h"
#include "filesys.h"
#include "sched_msgs.h"
#include "validate_util.h"
#include "sched_config.h"

#include "../util/util.hpp"
#include "../searches/particle_swarm.hpp"

using namespace std;

//returns 0 on sucess
int assimilate_handler(WORKUNIT& wu, vector<RESULT>& /*results*/, RESULT& canonical_result) {
    double canonical_fitness;
    try {
        canonical_fitness = parse_xml<double>(canonical_result.stderr_out, "search_likelihood", atof);
    } catch (string error_message) {
        log_messages.printf(MSG_CRITICAL, "ea_assimilation_policy assimilate_handler([RESULT#%d %s]) failed with TERMINAL error: could not parse canonical result xml\n", canonical_result.id, canonical_result.name);
        log_messages.printf(MSG_CRITICAL, "error message: %s", error_message.c_str());
        exit(0);
    }

    int particle_swarm_id;
    int position;
    try {
        particle_swarm_id = parse_xml<int>(wu.xml_doc, "particle_swarm_id", atoi);
        position = parse_xml<int>(wu.xml_doc, "position", atoi);
    } catch (string error_message) {
        log_messages.printf(MSG_CRITICAL, "ea_assimilation_policy assimilate_handler([WORKUNIT#%d %s]) failed with TERMINAL error: could not parse canonical result xml\n", wu.id, wu.name);
        log_messages.printf(MSG_CRITICAL, "error message: %s", error_message.c_str());
        exit(0);
    }

    /**
     *  Need to handle the case if the particle isn't in the database yet
     */
    Particle *particle = new Particle(boinc_db.mysql, particle_swarm_id, position);

    if (!particle->get_in_database() || particle->fitness < canonical_fitness) {
        //update fitness, local_best, velocity, metadata

        vector<double> *new_velocity = string_to_vector<double>(parse_xml<string>(wu.xml_doc, "velocity", convert_string), atof);
        vector<double> *new_local_best = string_to_vector<double>(parse_xml<string>(wu.xml_doc, "parameters", convert_string), atof);
        string metadata = parse_xml<string>(wu.xml_doc, "metadata", convert_string);

        particle->update_and_commit(boinc_db.mysql, canonical_fitness, new_local_best, new_local_best, new_velocity, metadata);
    }

    delete particle;

    increment_ps_evaluations_and_commit(boinc_db.mysql, particle_swarm_id);
}
