// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// sample_work_generator.cpp: an example BOINC work generator.
// This work generator has the following properties
// (you may need to change some or all of these):
//
// - Runs as a daemon, and creates an unbounded supply of work.
//   It attempts to maintain a "cushion" of 100 unsent job instances.
//   (your app may not work this way; e.g. you might create work in batches)
// - Creates work for the application "example_app".
// - Creates a new input file for each job;
//   the file (and the workunit names) contain a timestamp
//   and sequence number, so that they're unique.

#include <unistd.h>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sstream>

#include "boinc_db.h"
#include "config.h"
#include "error_numbers.h"
#include "backend_lib.h"
#include "parse.h"
#include "util.h"
#include "svn_version.h"

#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "str_util.h"

#include "particle_swarm.hpp"
#include "./mersenne_twister/dSFMT.h"

#define CUSHION 10
    // maintain at least this many unsent results
#define REPLICATION_FACTOR  1

using namespace std;

char* app_name = NULL;
char* in_template_file = NULL;
char* out_template_file = NULL;

char* in_template;
DB_APP app;
int start_time;
int seqno;

// create one new job
//
int make_jobs(int number_jobs) {
    DB_WORKUNIT wu;
    char name[256], path[256];
    int retval;

    ostringstream oss;
    oss << "SELECT * FROM particle_swarm WHERE app_id = " << app.id;
    cout << oss.str() << endl;
    mysql_query(boinc_db.mysql, oss.str().c_str());

    MYSQL_RES *result = mysql_store_result(boinc_db.mysql);

    if (!result) return 0;      //no searches to generate workunits for, for this application

    MYSQL_ROW row;
    vector<ParticleSwarm*> particle_swarms;
    vector<bool> search_completed;
    int incomplete_searches = 0;
    while ((row = mysql_fetch_row(result))) {
        ParticleSwarm *ps = new ParticleSwarm(boinc_db.mysql, row);
        particle_swarms.push_back(ps);

//        cout << "created particle swarm: " << ps->toString() << endl;

        if (ps->get_max_evaluations() == 0 || ps->get_max_evaluations() > ps->get_evaluations_done()) {
            search_completed.push_back(false);
            incomplete_searches++;
        } else {
            search_completed.push_back(true);
        }
    }

    cout << endl << endl;
    cout << "READ " << particle_swarms.size() << " PARTICLE SWARMS!" << endl;
    cout << "\t" << incomplete_searches << " searches are incomplete" << endl;
    cout << endl << endl;

    for (int i = 0; i < particle_swarms.size(); i++) {
        if (search_completed[i]) continue;
        
        int workunits_to_generate = number_jobs/incomplete_searches;
        cout << "Generating " << workunits_to_generate << " workunits for search " << particle_swarms[i]->get_name() << endl;

        // make a unique name (for the job and its input file)
        sprintf(name, "%s_%d_%d", app_name, start_time, seqno++);

        // Fill in the job parameters
        wu.clear();
        wu.appid = app.id;
        strcpy(wu.name, name);
        double credit;
        try {
            wu.rsc_fpops_est = parse_xml<double>(particle_swarms[i]->get_extra_xml(), "rsc_fpops_est", atof);
            wu.rsc_fpops_bound = parse_xml<double>(particle_swarms[i]->get_extra_xml(), "rsc_fpops_bound", atof);
            wu.rsc_disk_bound = parse_xml<double>(particle_swarms[i]->get_extra_xml(), "rsc_disk_bound", atof);
            credit = parse_xml<double>(particle_swarms[i]->get_extra_xml(), "credit", atof);
        } catch (string error_string) {
            cout << "Error parsing particle swarm extra xml: " << error_string << endl;
            cout << "particle_swarms[" << i << "]->get_extra_xml(): '" + particle_swarms[i]->get_extra_xml() << "'" << endl;
        }
        cout << "wu.rsc_fpops_est = " << wu.rsc_fpops_est << endl;
        cout << "wu.rsc_fpops_bound = " << wu.rsc_fpops_bound << endl;
        cout << "wu.rsc_disk_bound = " << wu.rsc_disk_bound << endl;
        cout << "credit = " << credit << endl;

//        wu.delay_bound = 86400;
//        wu.min_quorum = REPLICATION_FACTOR;
//        wu.target_nresults = REPLICATION_FACTOR;
//        wu.max_error_results = REPLICATION_FACTOR*4;
//        wu.max_total_results = REPLICATION_FACTOR*8;
//        wu.max_success_results = REPLICATION_FACTOR*4;

        vector<string>* input_filenames = particle_swarms[i]->get_input_filenames();
        const char* infiles[input_filenames->size()];
        for (int j = 0; j < input_filenames->size(); j++) {
            infiles[j] = input_filenames->at(j).c_str();
            cout << "infiles[" << j << "]: " << infiles[j] << endl;
            retval = config.download_path(infiles[j], path);

            cout << "will put file at: " << path << endl;

        //    if (retval) return retval;
        //    FILE* f = fopen(path, "w");
        //    if (!f) return ERR_FOPEN;
        //    fprintf(f, "This is the input file for job %s", name);
        //    fclose(f);
        }

        oss.str("");

        sprintf(path, "templates/%s", out_template_file);
        cout << "result_template_filename: " << path << endl;
        cout << "result_template_filepath: " << config.project_path(path) << endl;

        vector<Particle*> *particles = particle_swarms[i]->generate_new_particles(boinc_db.mysql, workunits_to_generate);
        for (int j = 0; j < workunits_to_generate; j++) {
            Particle *particle = particles->at(j);

            vector<double> *parameters = particle->parameters;
            oss.str("");
            oss << "-np " << parameters->size() << " -p";
            for (int k = 0; k < parameters->size(); k++) {
                oss << " " << parameters->at(k);
            }
            string command_line = oss.str();

            cout << endl << "command line: '" << command_line.c_str() << "'" << endl;

            oss.str("");
            oss << "<credit>" << credit << "</credit>";
            oss << "<particle_swarm_id>" << particle_swarms[i]->get_id() << "</particle_swarm_id>";
            oss << "<position>" << particle->position << "</position>";
            oss << "<parameters>" << vector_to_string<double>(parameters) << "</parameters>";
            oss << "<velocity>" << vector_to_string<double>(particle->velocity) << "</velocity>";
            oss << "<metadata>" << particle->metadata << "</metadata>";

            string additional_xml = oss.str();

            cout << "additional xml: '" << additional_xml.c_str() << "'" << endl;

            // Register the job with BOINC
        //    return create_work(
        //        wu,
        //        in_template,
        //        path,
        //        config.project_path(path),
        //        infiles,
        //        input_filenames.size(),
        //        config,
        //        command_line.c_str(),
        //        additional_xml.c_str()
        //    );
        }

        cout << "particles alive: " << particles_alive << endl;

        while (!particles->empty()) {
            delete particles->back();
            particles->pop_back();
        }
        delete particles;
    }

    cout << "particles alive: " << particles_alive << endl;

    while (!particle_swarms.empty()) {
        delete particle_swarms.back();
        particle_swarms.pop_back();
    }

    cout << "particles alive: " << particles_alive << endl;
    return 0;
}

void main_loop() {
    int retval;

    while (1) {
        check_stop_daemons();
        int n;
        retval = count_unsent_results(n, 0);
        cout << "unsent results: " << n << endl;

//        if (n > CUSHION) {
//            sleep(10);
//        } else {
//            int njobs = (CUSHION-n)/REPLICATION_FACTOR;
            int njobs = 500;
            log_messages.printf(MSG_DEBUG, "Making %d jobs\n", njobs );
            make_jobs(njobs);

           // Now sleep for a few seconds to let the transitioner
            // create instances for the jobs we just created.
            // Otherwise we could end up creating an excess of jobs.
            sleep(5);
//        }
    }
}

void usage(char *name) {
    fprintf(stderr, "This is an example BOINC work generator.\n"
        "This work generator has the following properties\n"
        "(you may need to change some or all of these):\n"
        "- Runs as a daemon, and creates an unbounded supply of work.\n"
        "  It attempts to maintain a \"cushion\" of 100 unsent job instances.\n"
        "  (your app may not work this way; e.g. you might create work in batches)\n"
        "- Creates work for the application \"example_app\".\n"
        "- Creates a new input file for each job;\n"
        "  the file (and the workunit names) contain a timestamp\n"
        "  and sequence number, so that they're unique.\n\n"
        "Usage: %s [OPTION]...\n\n"
        "Options:\n"
        "  [ --app X                Application name (default: example_app)\n"
        "  [ --in_template_file     Input template (default: example_app_in)\n"
        "  [ --out_template_file    Output template (default: example_app_out)\n"
        "  [ -d X ]                 Sets debug level to X.\n"
        "  [ -h | --help ]          Shows this help text.\n"
        "  [ -v | --version ]       Shows version information.\n",
        name
    );
}

int main(int argc, char** argv) {
    int i, retval;
    char buf[256];

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
        } else if (!strcmp(argv[i], "--in_template_file")) {
            in_template_file = argv[++i];
        } else if (!strcmp(argv[i], "--out_template_file")) {
            out_template_file = argv[++i];
        } else if (is_arg(argv[i], "h") || is_arg(argv[i], "help")) {
            usage(argv[0]);
            exit(0);
        } else if (is_arg(argv[i], "v") || is_arg(argv[i], "version")) {
            printf("%s\n", SVN_VERSION);
            exit(0);
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

    if (app_name == NULL) {
        log_messages.printf(MSG_CRITICAL, "app_name not specified\n");
        exit(1);
    }
    sprintf(buf, "where name='%s'", app_name);
    if (app.lookup(buf)) {
        log_messages.printf(MSG_CRITICAL, "can't find app %s\n", app_name);
        exit(1);
    }

    sprintf(buf, "templates/%s", in_template_file);
    if (read_file_malloc(config.project_path(buf), in_template)) {
        log_messages.printf(MSG_CRITICAL, "can't read input template %s\n", buf);
        exit(1);
    }

    if (out_template_file == NULL) {
        log_messages.printf(MSG_CRITICAL, "out (result) template file not specified, use --out_template_file <name>\n");
        exit(1);
    }

    start_time = time(0);
    seqno = 0;

    log_messages.printf(MSG_NORMAL, "Starting\n");

    dsfmt_gv_init_gen_rand((int)time(NULL));

    main_loop();
}
