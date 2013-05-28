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

#include "config.h"
#include <vector>
#include <cstdlib>
#include <string>
#include <map>
#include <limits>

#include "boinc_db.h"
#include "error_numbers.h"

#include "credit.h"
#include "sched_config.h"
#include "sched_msgs.h"
#include "sched_util.h"

#include "validator.h"
#include "validate_util.h"
#include "validate_util2.h"

#include "asynchronous_algorithms/particle_swarm_db.hxx"
#include "asynchronous_algorithms/differential_evolution_db.hxx"

#include "undvc_common/vector_io.hxx"
#include "undvc_common/parse_xml.hxx"

#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"

#define FITNESS_ERROR_BOUND 10e-8


map<string, EvolutionaryAlgorithm*> searches;

using boost::variate_generator;
using boost::mt11213b;
using boost::uniform_real;

variate_generator< mt11213b, uniform_real<> > random_number_generator(mt11213b( time(0) ), uniform_real<>(0.0, 1.0));

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

    /**
     *  Parse all the fitnesses from the results
     */
    vector<bool> had_error(results.size(), false);
    vector<double> result_fitness(results.size(), -std::numeric_limits<double>::max());
    for (uint32_t i = 0; i < results.size(); i++) {
        try {
            result_fitness[i] = parse_xml<double>(results[i].stderr_out, "search_likelihood");
        } catch (string error_message) {
            log_messages.printf(MSG_CRITICAL, "ea_validation_policy check_set([RESULT#%d %s]) failed with error: %s\n", results[i].id, results[i].name, error_message.c_str());
            had_error[i] = true;

            results[i].outcome = RESULT_OUTCOME_VALIDATE_ERROR;
            results[i].validate_state = VALIDATE_STATE_INVALID;
        }
    }

    /**
     *  Get workunit based information (search name, id, individual position, parameters)
     */

    //need to read wu.xml_doc
    string xml_doc;

    ostringstream oss;
    oss << "SELECT xml_doc FROM workunit WHERE id = " << wu.id;
    string query = oss.str();

    mysql_query(boinc_db.mysql, query.c_str());

    MYSQL_RES *my_result = mysql_store_result(boinc_db.mysql);
    if (mysql_errno(boinc_db.mysql) == 0) {
        MYSQL_ROW row = mysql_fetch_row(my_result);

        if (row == NULL) {
            log_messages.printf(MSG_CRITICAL, "Could not get row from workunit with query '%s'. Error: %d -- '%s'\n", xml_doc.c_str(), mysql_errno(boinc_db.mysql), mysql_error(boinc_db.mysql));
            exit(1);
        }

        xml_doc = row[0];
    } else {
        log_messages.printf(MSG_CRITICAL, "Could execute query '%s'. Error: %d -- '%s'\n", xml_doc.c_str(), mysql_errno(boinc_db.mysql), mysql_error(boinc_db.mysql));
        exit(1);
    }
    mysql_free_result(my_result);

    string search_name;
    uint32_t search_id;
    uint32_t position = 0;
    vector<double> result_parameters;

    try {
        position = parse_xml<int>(xml_doc, "position");
        parse_xml_vector<double>(xml_doc, "parameters", result_parameters);
    } catch (string error_message) {
        log_messages.printf(MSG_CRITICAL, "ea_validation_policy check_set([WORKUNIT#%d %s]) failed with error: %s\n", wu.id, wu.name, error_message.c_str());
        exit(1);
    }

    try {
        search_id = parse_xml<int>(xml_doc, "search_id");
        search_name = parse_xml<string>(xml_doc, "search_name");
    } catch (string error_message) {
        log_messages.printf(MSG_CRITICAL, "ea_validation_policy check_set([WORKUNIT#%d %s]) failed with error: %s\n", wu.id, wu.name, error_message.c_str());

        //This search has been removed from the database (or was from before this update)
        search_id = -1;
        search_name = "unknown";
    }

//    cout << "parameters: " << vector_to_string(result_parameters) << endl;
//    cout << "position: " << position << endl;
//    cout << "search_id: " << search_id << endl;
//    cout << "search_name: " << search_name << endl;

    //Get the cached evolutionary algorithm if possible
    EvolutionaryAlgorithm *ea = NULL;
    try {
        if (searches.find(search_name) == searches.end()) {
            log_messages.printf(MSG_DEBUG, "search '%s' not found, looking up in database.\n", search_name.c_str());

            if (search_name.substr(0,3).compare("ps_") == 0) {
                ea = new ParticleSwarmDB(boinc_db.mysql, search_id);
                searches[search_name] = ea;
            } else if (search_name.substr(0,3).compare("de_") == 0) {
                ea = new DifferentialEvolutionDB(boinc_db.mysql, search_id);
                searches[search_name] = ea;
            } else {
                log_messages.printf(MSG_CRITICAL, "ea_validation_policy check_set([WORKUNIT#%d %s]) had an unkown search name (either removed from database or needs to start with de_ or ps_): '%s'\n", wu.id, wu.name, search_name.c_str());
                ea = NULL;
            }

            if (ea != NULL) {
                const char *log_file_path = config.project_path("search_progress/%s", search_name.c_str());

                log_messages.printf(MSG_DEBUG, "Opening log file for search '%s': '%s'\n", search_name.c_str(), log_file_path);

                /**
                 * Should write to database instead
                 */
                ea->set_log_file( new ofstream(log_file_path, fstream::app) );
            }
        } else {
            ea = searches[search_name];
        }
    } catch (string err_msg) {
        log_messages.printf(MSG_CRITICAL, "could not find search: '%s' from search id '%d', threw error: '%s\n", search_name.c_str(), search_id, err_msg.c_str());
        ea = NULL;
    }

    if (results.size() == 1) {  //This was the first result back for the workunit
        if (had_error[0]) {
            log_messages.printf(MSG_CRITICAL, "ea_validation_policy check_set([RESULT#%d %s]) had an error was marked as invalid, and there are no other results for this workunit.\n", results[0].id, results[0].name);
//            log_messages.printf(MSG_CRITICAL, "wu.xml_doc:\n%s\n\n", xml_doc.c_str());
//            log_messages.printf(MSG_CRITICAL, "result.stderr_out:\n%s\n\n", results[0].stderr_out);
//            exit(1);
            return 0;   //This result had an error and was marked invalid
        }

        if (ea == NULL || !ea->would_insert(position, result_fitness[0])) {  //This is a result we aren't going to use, so check to see if we need to use adaptive validation, and just mork it valid otherwise
            //Use adaptive validation to determine if we need to validate the result
            DB_HOST_APP_VERSION hav;
            int retval = hav_lookup(hav, results[0].hostid, generalized_app_version_id(results[0].app_version_id, results[0].appid));

            //retval == 0 means a successful lookup
            if (!retval) {
                double success_rate = (((double)hav.consecutive_valid) / 10.0);
                if (success_rate > 0.9) success_rate = 0.9;
                else if (success_rate < 0) success_rate = 0; 

                log_messages.printf(MSG_DEBUG, "host application version consecutive valid: %d, success_rate: %lf\n", hav.consecutive_valid, success_rate);

                //use adaptive validation
                double r_num = random_number_generator();
                if (r_num < success_rate) {
                    log_messages.printf(MSG_DEBUG, "ea_validation_policy check_set([RESULT#%d %s]), single result, r_num %lf < success_rate %lf, marking valid.\n", results[0].id, results[0].name, r_num, success_rate);
                    results[0].validate_state = VALIDATE_STATE_VALID;
                    canonicalid = results[0].id;
                } else {
                    log_messages.printf(MSG_DEBUG, "ea_validation_policy check_set([RESULT#%d %s]), single result, r_num %lf >= success_rate %lf, marking inconclusive.\n", results[0].id, results[0].name, r_num, success_rate);
                }
            } else {
                    log_messages.printf(MSG_DEBUG, "ea_validation_policy check_set([RESULT#%d %s]), single result, going to be inserted into EA, marking inconclusive for validation.\n", results[0].id, results[0].name);
            }
            //otherwise the result is marked inconclusive and will be resent for validation (because we would insert into an evolutionary algorithm)

            ostringstream oss;
            oss << "UPDATE workunit SET min_quorum = min_quorum + 1 WHERE id = " << wu.id;
            mysql_query(boinc_db.mysql, oss.str().c_str());

            if (mysql_errno(boinc_db.mysql) != 0) {
                log_messages.printf(MSG_CRITICAL, "ea_validation_policy check_set([WORKUNIT#%d %s]) could not update workunit min_quorum field. Error: %d -- '%s'\n", wu.id, wu.name, mysql_errno(boinc_db.mysql), mysql_error(boinc_db.mysql));
                exit(1);
            }

//            wu.min_quorum++;    //may be able to just do this
        }
    } else {
        /**
         *  Determine which result is canonical
         */

        int min_quorum = wu.min_quorum;
        double canonical_fitness;
        int canonical_resultid = -1;

        int matches;

        for (uint32_t i = 0; i < results.size(); i++) {
            if (had_error[i]) continue;

            matches = 0;
            for (uint32_t j = 0; j < results.size(); j++) {
                if (had_error[j]) continue;
                if (i == j) continue;

                if (fabs(result_fitness[i] - result_fitness[j]) < FITNESS_ERROR_BOUND) {
                    matches++;
                }
            }

            if (matches >= min_quorum) {
                canonical_fitness = result_fitness[i];
                canonical_resultid = results[i].id;
                break;
            }
        }

        if (canonical_resultid > 0) {
            canonicalid = canonical_resultid;

            log_messages.printf(MSG_DEBUG, "    min_quorum: %d, matches: %d, canonical_resultid: %d\n", min_quorum, matches, canonical_resultid);
            log_messages.printf(MSG_DEBUG, "    canonical_fitness: %lf\n", canonical_fitness);
            //We found a canonical result so mark the valid results valid the the others invalid
            for (uint32_t i = 0; i < results.size(); i++) {
                if (fabs(result_fitness[i] - canonical_fitness) < FITNESS_ERROR_BOUND) {
                    results[i].validate_state = VALIDATE_STATE_VALID;
                    log_messages.printf(MSG_DEBUG, "              fitness: %lf -- marked valid,   result_id: %d.\n", result_fitness[i], results[i].id);
                } else {
                    results[i].outcome = RESULT_OUTCOME_VALIDATE_ERROR;
                    results[i].validate_state = VALIDATE_STATE_INVALID;
                    log_messages.printf(MSG_DEBUG, "              fitness: %lf -- marked INVALID, result_id: %d.\n", result_fitness[i], results[i].id);
                }
            }

            /**
             *  Insert the fitness of the canonical result into the EA
             */
            if (ea != NULL) {
                try {
                    uint32_t seed = parse_xml<uint32_t>(xml_doc, "seed");
                    ea->insert_individual(position, result_parameters, canonical_fitness, seed);
                } catch (string error_message) {
//                    log_messages.printf(MSG_DEBUG, "Inserting individual threw error: '%s'\n", error_message.c_str());

                    //can ignore if the seed is not there since it's optional
                    try {
                        ea->insert_individual(position, result_parameters, canonical_fitness);
                    } catch (string error_message2) {
                        log_messages.printf(MSG_CRITICAL, "Error inserting individual: %s\n", error_message2.c_str());
                        exit(1);
                    }
                }
            }
        }
    }
    return 0;
}

void check_pair(RESULT& new_result, RESULT& canonical_result, bool& retry) {
    double new_fitness, canonical_fitness;

    try {
        new_fitness = parse_xml<double>(new_result.stderr_out, "search_likelihood");
    } catch (string error_message) {
        log_messages.printf(MSG_CRITICAL, "ea_validation_policy check_pair([RESULT#%d %s]) failed with error: %s\n", new_result.id, new_result.name, error_message.c_str());
        new_result.outcome = RESULT_OUTCOME_VALIDATE_ERROR;
        new_result.validate_state = VALIDATE_STATE_INVALID;
        return;
    }

    try {
        canonical_fitness = parse_xml<double>(canonical_result.stderr_out, "search_likelihood");
    } catch (string error_message) {
        log_messages.printf(MSG_CRITICAL, "ea_validation_policy check_set([RESULT#%d %s]) failed with TERMINAL error: %s -- COULD NOT PARSE CANONICAL RESULT XML\n", canonical_result.id, canonical_result.name, error_message.c_str());
        exit(1);
    }

    if (fabs(new_fitness - canonical_fitness) < FITNESS_ERROR_BOUND) {
        new_result.validate_state = VALIDATE_STATE_VALID;
    } else {
        new_result.validate_state = VALIDATE_STATE_INVALID;
    }

//    cout << "CHECK PAIR SUCCESS!" << endl;
//    exit(1);
}

