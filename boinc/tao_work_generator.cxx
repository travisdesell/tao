/*
 * Copyright 2012, 2009 Travis Desell and the University of North Dakota.
 *
 * This file is part of the Toolkit for Asynchronous Optimization (TAO)
 * and is modified from the original within the BOINC source code (also 
 * GPL 3).
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


#include <sys/param.h>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <map>

#include "boinc_db.h"
#include "error_numbers.h"
#include "backend_lib.h"
#include "parse.h"
#include "util.h"
#include "svn_version.h"

#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "str_util.h"

#include "stdint.h"

#include "undvc_common/parse_xml.hxx"
#include "undvc_common/vector_io.hxx"

#include "asynchronous_algorithms/evolutionary_algorithm.hxx"
#include "asynchronous_algorithms/particle_swarm_db.hxx"
#include "asynchronous_algorithms/differential_evolution_db.hxx"

#include "workunit_information.hxx"

#ifdef FPOPS_FROM_PARAMETERS
#include "fpops_from_parameters.hxx"
#endif


#define CUSHION 500 // maintain at least this many unsent results
#define REPLICATION_FACTOR  1

using std::ostringstream;
using std::vector;

const char* app_name = "example_app";

DB_APP app;
int start_time;
int seqno;

bool requires_seeding = false;

map<string, char*> in_templates;

// create one new job
//
int make_job(string search_name,
             string workunit_xml_filename,
             string result_xml_filename,
             vector<string> input_filenames,
             string command_line_options,
             string extra_xml,
             uint32_t search_id) {

    DB_WORKUNIT wu;
    char name[256], path[MAXPATHLEN];

    // make a unique name (for the job and its input file)
    sprintf(name, "%s_%d_%d", search_name.c_str(), start_time, seqno++);

    // Fill in the job parameters
    wu.clear();
    wu.appid = app.id;
    strcpy(wu.name, name);
//  These should be filled in by the extra xml in the workunit_information
//    wu.rsc_fpops_est = 1e12;
//    wu.rsc_fpops_bound = 1e14;
//    wu.rsc_memory_bound = 1e8;
//    wu.rsc_disk_bound = 1e8;
//    wu.delay_bound = 86400;
    wu.min_quorum = REPLICATION_FACTOR;
    wu.target_nresults = REPLICATION_FACTOR;
    wu.max_error_results = REPLICATION_FACTOR*4;
    wu.max_total_results = REPLICATION_FACTOR*8;
    wu.max_success_results = REPLICATION_FACTOR*4;

    wu.batch = search_id;

    try {
        wu.rsc_fpops_est    = parse_xml<double>(extra_xml, "rsc_fpops_est");
        wu.rsc_fpops_bound  = parse_xml<double>(extra_xml, "rsc_fpops_bound");
        wu.rsc_disk_bound   = parse_xml<double>(extra_xml, "rsc_disk_bound");
    } catch (string err_msg) {
        log_messages.printf(MSG_CRITICAL, "ERROR: parsing extra xml for workunit field: '%s'\n", err_msg.c_str());
        exit(1);
    }

    const char* infiles[input_filenames.size()];
    for (uint32_t i = 0; i < input_filenames.size(); i++) {
        infiles[i] = input_filenames[i].c_str();
//        cout << "input file[" << i << "]: " << infiles[i] << endl;
    }

    char *in_template;
    if (in_templates.find(workunit_xml_filename) == in_templates.end()) {
        // Buffer the input template
        char buf[256];
        sprintf(buf, "templates/%s", workunit_xml_filename.c_str());
        if (read_file_malloc(config.project_path(buf), in_template)) {
            log_messages.printf(MSG_CRITICAL, "can't read input template %s\n", buf);
            exit(1);
        }
        in_templates[workunit_xml_filename] = in_template;
    } else {
        in_template = in_templates[workunit_xml_filename];  //use the buffered template
    }

//    cout << "extra_xml: " << extra_xml << endl;
//    cout << "command_line_options: " << command_line_options << endl;

    // Register the job with BOINC
    sprintf(path, "templates/%s", result_xml_filename.c_str());
    return create_work(
        wu,
        in_template,
        path,
        config.project_path(path),
        infiles,
        input_filenames.size(),
        config,
        command_line_options.c_str(),
        extra_xml.c_str()
    );
}

/**
 *  Get searches from databse
 *  Determine which searches are not finished
 *
 *  For each unfinished search:
 *      Get workunit information for search
 *      generate equal portion of workunits
 */
int make_jobs(uint32_t number_jobs) {
    int retval;

    vector<EvolutionaryAlgorithmDB*> unfinished_searches;
    try {
        log_messages.printf(MSG_DEBUG, "getting unfinished particle swarms\n");
        ParticleSwarmDB::add_unfinished_searches(boinc_db.mysql, app.id, unfinished_searches);
        log_messages.printf(MSG_DEBUG, "getting unfinished differential_evolutions\n");
        DifferentialEvolutionDB::add_unfinished_searches(boinc_db.mysql, app.id, unfinished_searches);
    } catch (string err_msg) {
        log_messages.printf(MSG_CRITICAL, "Error thrown getting unfinished searches:\n    '%s'\n", err_msg.c_str());
        exit(1);
    }

    log_messages.printf(MSG_DEBUG, "got %lu unfinished searches\n", unfinished_searches.size());
    log_messages.printf(MSG_DEBUG, "number jobs: %u\n", number_jobs);

    if (unfinished_searches.size() == 0) return 0;
    uint64_t portion = (uint64_t)number_jobs / unfinished_searches.size();

    log_messages.printf(MSG_DEBUG, "Generating %u total jobs for %lu unfinished searches.\n", number_jobs, unfinished_searches.size());

    /**
     *  For each unfinished search generate equal portion of workunits
     */
    for (uint32_t i = 0; i < unfinished_searches.size(); i++) {
        log_messages.printf(MSG_DEBUG, "    Generating %lu jobs for unfinished search '%s'.\n", portion, unfinished_searches[i]->get_name().c_str());

        /**
         *  Get the standard workunit information for this search
         */
        WorkunitInformation workunit_information(boinc_db.mysql, unfinished_searches[i]->get_name());       //TODO: should cache this since it never changes
//        cout << "info: " << workunit_information << endl;

        for (uint32_t j = 0; j < portion; j++) {
//            log_messages.printf(MSG_DEBUG, "        JOB %u\n", j);
            uint32_t id;
            uint32_t seed;

            vector<double> parameters;
            try {
                if (requires_seeding) unfinished_searches[i]->new_individual(id, parameters, seed);
                else unfinished_searches[i]->new_individual(id, parameters);
            } catch (string err_msg) {
                log_messages.printf(MSG_CRITICAL, "ERROR: creating new individual for search '%s' threw error message: '%s'.\n", unfinished_searches[i]->get_name().c_str(), err_msg.c_str());
                exit(1);
            }

            ostringstream new_command_line;
            new_command_line << workunit_information.get_command_line_options();
            if (requires_seeding) new_command_line << " --seed " << seed;
            new_command_line << " -np " << parameters.size() << " -p";
            new_command_line.precision(15);
            for (uint32_t k = 0; k < parameters.size(); k++) new_command_line << " " << parameters[k];

            ostringstream new_extra_xml;
            new_extra_xml << workunit_information.get_extra_xml() << endl;
#ifdef FPOPS_FROM_PARAMETERS
            double rsc_fpops_est, rsc_fpops_bound;
            calculate_fpops(parameters, rsc_fpops_est, rsc_fpops_bound, workunit_information.get_extra_xml());
            new_extra_xml << "<rsc_fpops_est>"   << rsc_fpops_est   << "</rsc_fpops_est>"   << endl;
            new_extra_xml << "<rsc_fpops_bound>" << rsc_fpops_bound << "</rsc_fpops_bound>" << endl;
#endif
            new_extra_xml << "<search_name>" << unfinished_searches[i]->get_name() << "</search_name>" << endl;
            new_extra_xml << "<search_id>" << unfinished_searches[i]->get_id() << "</search_id>" << endl;
            new_extra_xml << "<position>" << id << "</position>" << endl;
            new_extra_xml << "<parameters>" << vector_to_string(parameters) << "</parameters>" << endl;
            if (requires_seeding) new_extra_xml << "<seed>" << seed << "</seed>" << endl;

//            cerr << "new_extra_xml: " << endl;
//            cerr << new_extra_xml.str() << endl;

            /**
             *  Generate the job with the updated workunit information
             */
            retval = make_job(unfinished_searches[i]->get_name(),
                              workunit_information.get_workunit_xml_filename(),
                              workunit_information.get_result_xml_filename(),
                              workunit_information.get_input_filenames(),
                              new_command_line.str(),
                              new_extra_xml.str(),
                              unfinished_searches[i]->get_id()
                             );

            if (retval) return retval;
        }

        try {
            unfinished_searches[i]->update_current_individual();
        } catch (string err_msg) {
            log_messages.printf(MSG_CRITICAL, "ERROR: updating current individual for search '%s' threw error message: '%s'.\n", unfinished_searches[i]->get_name().c_str(), err_msg.c_str());
            exit(1);
        }
    }

    EvolutionaryAlgorithmDB *eadb;
    while (unfinished_searches.size() > 0) {
        eadb = unfinished_searches.back();
        unfinished_searches.pop_back();
        delete eadb;
    }

    return 0;
}

void main_loop() {
    int retval;

    while (1) {
        check_stop_daemons();

        int n;
        retval = count_unsent_results(n, app.id);

        if (retval) {
            log_messages.printf(MSG_CRITICAL,"count_unsent_jobs() failed: %s\n", boincerror(retval));
            exit(retval);
        }

        if (n > CUSHION) {
            sleep(30);
        } else {
            int njobs = (CUSHION - n)/REPLICATION_FACTOR;
            log_messages.printf(MSG_DEBUG, "Making %d jobs\n", njobs);

            retval = make_jobs(njobs);
            if (retval) {
                log_messages.printf(MSG_CRITICAL, "failed making jobs with error: %s\n", boincerror(retval));
                exit(retval);
            }

            // Now sleep for a few seconds to let the transitioner
            // create instances for the jobs we just created.
            // Otherwise we could end up creating an excess of jobs.
            sleep(30);
        }
    }
}

void usage(char *name) {
    fprintf(stderr, "This is an example BOINC work generator.\n"
        "This work generator has the following properties\n"
        "(you may need to change some or all of these):\n"
        "  It attempts to maintain a \"cushion\" of 100 unsent job instances.\n"
        "  (your app may not work this way; e.g. you might create work in batches)\n"
        "- Creates work for the application \"example_app\".\n"
        "- Creates a new input file for each job;\n"
        "  the file (and the workunit names) contain a timestamp\n"
        "  and sequence number, so that they're unique.\n\n"
        "Usage: %s [OPTION]...\n\n"
        "Options:\n"
        "  [ --app X                Application name (default: example_app)\n"
        "  [ -d X ]                 Sets debug level to X.\n"
        "  [ -h | --help ]          Shows this help text.\n"
        "  [ -v | --version ]       Shows version information.\n"
        "  [ -c | --create-table ]  Create the database table 'tao_workunit_information' used to store workunit information.\n",
        name
    );
}

int main(int argc, char** argv) {
    int i, retval;
    char buf[256];
    bool create_table = false;

    for (i=1; i<argc; i++) {
        if (is_arg(argv[i], "d")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            int dl = atoi(argv[i]);
            log_messages.set_debug_level(dl);
            if (dl == 4) g_print_queries = true;
        } else if (!strcmp(argv[i], "--app")) {
            app_name = argv[++i];
        } else if (is_arg(argv[i], "h") || is_arg(argv[i], "help")) {
            usage(argv[0]);
            exit(0);
        } else if (is_arg(argv[i], "v") || is_arg(argv[i], "version")) {
            printf("%s\n", SVN_VERSION);
            exit(0);
        } else if (is_arg(argv[i], "c") || is_arg(argv[i], "create_table")) {
            create_table = true;
        } else if (is_arg(argv[i], "s") || is_arg(argv[i], "requires_seeding")) {
            requires_seeding = true;
        } else {
            log_messages.printf(MSG_CRITICAL, "unknown command line argument: %s\n\n", argv[i]);
            usage(argv[0]);
            exit(1);
        }
    }

    retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Can't parse config.xml: %s\n", boincerror(retval)
        );
        exit(1);
    }

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't open db\n");
        exit(1);
    }

    sprintf(buf, "where name='%s'", app_name);
    if (app.lookup(buf)) {
        log_messages.printf(MSG_CRITICAL, "can't find app %s\n", app_name);
        exit(1);
    }

    if (create_table) {
        WorkunitInformation::create_table(boinc_db.mysql);
        log_messages.printf(MSG_CRITICAL, "'tao_workunit_information' database table created successfully.\n");
        exit(0);
    }

    start_time = time(0);
    seqno = 0;

    log_messages.printf(MSG_NORMAL, "Starting\n");

    main_loop();
}
