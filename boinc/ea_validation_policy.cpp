#include "config.h"
#include <vector>
#include <cstdlib>
#include <string>

#include "boinc_db.h"
#include "error_numbers.h"

#include "credit.h"
#include "sched_config.h"
#include "sched_msgs.h"
#include "sched_util.h"

#include "validator.h"
#include "validate_util.h"
#include "validate_util2.h"

#include "../searches/db_particle_swarm.hpp"

#define FITNESS_ERROR_BOUND 10e-10

/*
 * Given a set of results, check for a canonical result,
 * i.e. a set of at least min_quorum/2+1 results for which
 * that are equivalent according to check_pair().
 *
 * invariants:
 * results.size() >= wu.min_quorum
 * for each result:
 *      result.outcome == SUCCESS
 *      result.validate_state == INIT
 */
int check_set(vector<RESULT>& results, WORKUNIT& wu, int& canonicalid, double&, bool& retry) {
    retry = false;

    if (results.size() == 1) {
        int particle_swarm_id;
        int position;
        double result_fitness;
        RESULT result = results[0];

        try {
            particle_swarm_id = parse_xml<int>(wu.xml_doc, "particle_swarm_id", atoi);
            position = parse_xml<int>(wu.xml_doc, "position", atoi);
            result_fitness = parse_xml<double>(result.stderr_out, "search_likelihood", atof);
        } catch (string error_message) {
            log_messages.printf(MSG_CRITICAL, "ea_validation_policy check_set([RESULT#%d %s]) failed with error: %s\n", result.id, result.name, error_message.c_str());
            result.outcome = RESULT_OUTCOME_VALIDATE_ERROR;
            result.validate_state = VALIDATE_STATE_INVALID;
            exit(0);
            return 0;
        }
        
        /**
         *  Need to check and see if the particle is not in the database
         */
        DBParticle *particle = new DBParticle(boinc_db.mysql, particle_swarm_id, position);

        if (particle->get_in_database() && result_fitness <= particle->get_fitness()) {
            DB_HOST_APP_VERSION hav;
            int retval = hav_lookup(hav, result.hostid, generalized_app_version_id(result.app_version_id, result.appid));

            //retval == 0 means a successful lookup
            if (!retval) {
                double error_rate = 1.0 - (((double)hav.consecutive_valid) / 10.0);
                if (error_rate > 0.9) error_rate = 1.0;
                else if (error_rate < 0.1) error_rate = 0.1;

                //use adaptive validation
                if (drand48() < error_rate) {
                    result.validate_state = VALIDATE_STATE_VALID;
                    canonicalid = result.id;
                }
            }
        }
	delete particle;
        //otherwise the result will stay inconclusive 
    } else {
        int min_quorum = wu.min_quorum;
        if (min_quorum <= 1) min_quorum = 2;    //It will start as 1 but we need to increase it because of adaptive validation

        int n_matches;
        double result_fitness_i, result_fitness_j;

        vector<bool> had_error(false, results.size());

        for (unsigned int i = 0; i < results.size(); i++) {
            if (had_error[i]) continue;
            vector<bool> matches;
            matches.resize(results.size());

            try {
                result_fitness_i = parse_xml<double>(results[i].stderr_out, "search_likelihood", atof);
            } catch (string error_message) {
                log_messages.printf(MSG_CRITICAL, "ea_validation_policy check_set([RESULT#%d %s]) failed with error: %s\n", results[i].id, results[i].name, error_message.c_str());
                results[i].outcome = RESULT_OUTCOME_VALIDATE_ERROR;
                results[i].validate_state = VALIDATE_STATE_INVALID;
                had_error[i] = true;
                continue;
            }

            cout << "results[" << i << "] fitness: " << result_fitness_i << endl;

            n_matches = 1;

            for (unsigned int j = 0; j < results.size(); j++) {
                if (had_error[j]) continue;

                if (i == j) {
                    matches[i] = true;
                    continue;
                }

                try {
                    result_fitness_j = parse_xml<double>(results[j].stderr_out, "search_likelihood", atof);
                } catch (string error_message) {
                    log_messages.printf(MSG_CRITICAL, "ea_validation_policy check_set([RESULT#%d %s]) failed with error: %s\n", results[j].id, results[j].name, error_message.c_str());
                    results[j].outcome = RESULT_OUTCOME_VALIDATE_ERROR;
                    results[j].validate_state = VALIDATE_STATE_INVALID;
                    matches[j] = false;
                    had_error[j] = true;
                    continue;
                }

                cout << "  results[" << j << "] fitness: " << result_fitness_j << endl;

                if (fabs(result_fitness_j - result_fitness_i) < FITNESS_ERROR_BOUND) {
                    n_matches++;
                    matches[j] = true;
                } else {
                    matches[j] = false;
                }
            }

            if (n_matches >= min_quorum) {
                cout << "found n_matches: " << n_matches << " which are greater than min_quorum: " << min_quorum << endl;

                canonicalid = results[i].id;

                for (unsigned int k = 0; k < results.size(); k++) {
                    if (had_error[k]) continue;

                    if (matches[k]) results[k].validate_state = VALIDATE_STATE_VALID;
                    else results[k].validate_state = VALIDATE_STATE_INVALID;
                }
                exit(0);
                return 0;
            }
        }
    }

    cout << "SUCCESS!" << endl;
    exit(0);
    return 0;
}

void check_pair(RESULT& new_result, RESULT& canonical_result, bool& retry) {
    double new_fitness, canonical_fitness;

    try {
        new_fitness = parse_xml<double>(new_result.stderr_out, "search_likelihood", atof);
    } catch (string error_message) {
        log_messages.printf(MSG_CRITICAL, "ea_validation_policy check_pair([RESULT#%d %s]) failed with error: %s\n", new_result.id, new_result.name, error_message.c_str());
        new_result.outcome = RESULT_OUTCOME_VALIDATE_ERROR;
        new_result.validate_state = VALIDATE_STATE_INVALID;
        return;
    }

    try {
        canonical_fitness = parse_xml<double>(canonical_result.stderr_out, "search_likelihood", atof);
    } catch (string error_message) {
        log_messages.printf(MSG_CRITICAL, "ea_validation_policy check_set([RESULT#%d %s]) failed with TERMINAL error: %s -- COULD NOT PARSE CANONICAL RESULT XML\n", canonical_result.id, canonical_result.name, error_message.c_str());
        exit(0);
    }

    if (fabs(new_fitness - canonical_fitness) < FITNESS_ERROR_BOUND) {
        new_result.validate_state = VALIDATE_STATE_VALID;
    } else {
        new_result.validate_state = VALIDATE_STATE_INVALID;
    }

    cout << "CHECK PAIR SUCCESS!" << endl;
    exit(0);
}

