#include <iostream>
#include <string>

#include "boinc_db.h"
#include "config.h"

using namespace std;

DB_APP app;


void usage(char *name) {
    fprintf(stderr, "This is will start an evolutionary algorithm.\n"
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


int main(int argc, char **argv) {
    string app_name;                    //need from command line arguments
    string particle_swarm_name;         //need from command line arguments
    int app_id;                         //should be generated
    int max_evaluations;                //need from command line arguments
    int requires_seeding;               //need from command line arguments
    double w, c1, c2;                   //need from command line arguments
    int number_particles;               //need from command line arguments
    vector<double> *min_bound;          //should be generated
    vector<double> *max_bound;          //should be generated
    string command_line_options;        //optional
    string workunit_xml_filename;       //need from command line arguments
    string result_xml_filename;         //need from command line arguments
    vector<string> *input_filenames;    //need from command line arguments
    string extra_xml;                   //should be generated

    w = 0.98;                           //default
    c1 = 2.0;                           //default
    c2 = 2.0;                           //default
    max_evaluations = -1;               //default
    number_particles = 300;             //default

    input_filenames = new vector<string>();

    retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "Can't parse config.xml: %s\n", boincerror(retval) );
        exit(1);
    }

    retval = boinc_db.open( config.db_name, config.db_host, config.db_user, config.db_passwd );
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't open db\n");
        exit(1);
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--app")) {
            app_name = string(argv[++i]);
            sprintf(buf, "where name='%s'", app_name.c_str());
            if (app.lookup(buf)) {
                log_messages.printf(MSG_CRITICAL, "can't find app %s\n", app_name);
                exit(1);
            }

        } else if (strcmp(argv[i], "--particle_swarm_name")) {
            particle_swarm_name = string(argv[++i]);

        } else if (strcmp(argv[i], "--in_template_file")) {
            workunit_xml_filename = string(argv[++i]);

        } else if (strcmp(argv[i], "--out_template_file")) {
            result_xml_filename = string(argv[++i]);

        } else if (strcmp(argv[i], "--max_evaluations")) {
            max_valuations = atoi(argv[++i]);

        } else if (strcmp(argv[i], "--requires_seeding")) {
            requires_seeding = atoi(argv[++i]);

        } else if (strcmp(argv[i], "--w")) {
            w = atof(argv[++i]);

        } else if (strcmp(argv[i], "--c1")) {
            c1 = atof(argv[++i]);

        } else if (strcmp(argv[i], "--c2")) {
            c2 = atof(argv[++i]);

        } else if (strcmp(argv[i], "--number_particles")) {
            number_particles = atoi(argv[++i]);

        } else if (strcmp(argv[i], "--star_file")) {
            input_filenames->push_back( string(argv[++i]) );
        } else if (strcmp(argv[i], "--parameter_file")) {
            input_filenames->push_back( string(argv[++i]) );
        }
    }
    
    if (app_name == NULL) {
        log_messages.printf(MSG_CRITICAL, "app_name not specified\n");
        exit(1);
    }

    ParticleSwarm *particle_swarm = new ParticleSwarm(boinc_db.mysql, name, app.id, max_evaluations, requires_seeding, w, c1, c2, number_particles, min_bound, max_bound, command_line_options, workunit_xml_filename, result_xml_filename, input_filenames, extra_xml);

    mysql_close(conn);

}
