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

#ifndef TAO_WORKUNIT_INFORMATION
#define TAO_WORKUNIT_INFORMATION

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

class WorkunitInformation {
    protected:
        string search_name;
        int app_id;

        string workunit_xml_filename;
        string result_xml_filename;
        vector<string> input_filenames;
        string command_line_options;
        string extra_xml;

    public:
        string          get_workunit_xml_filename()     { return workunit_xml_filename; }
        string          get_result_xml_filename()       { return result_xml_filename; }
        vector<string>  get_input_filenames()           { return input_filenames; }
        string          get_command_line_options()      { return command_line_options; }
        string          get_extra_xml()                 { return extra_xml; }

        WorkunitInformation(MYSQL *conn,
                            const string search_name 
                           ) throw (string);

        WorkunitInformation(MYSQL *conn,
                            const string search_name,
                            const int app_id,
                            const string workunit_xml_filename,
                            const string result_xml_filename,
                            const vector<string> &input_filenames,
                            const string command_line_options,
                            const string extra_xml
                           ) throw (string);

        static void create_table(MYSQL *conn) throw (string);

        void print_to(ostream& stream);
        friend std::ostream& operator<< (std::ostream& stream, WorkunitInformation &wu_info);
};


#endif
