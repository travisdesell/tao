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

#include <string>
#include <vector>
#include <sstream>
#include <iostream>

#include "stdlib.h"

#include "undvc_common/vector_io.hxx"

#include "mysql.h"
#include "workunit_information.hxx"

using std::string;
using std::vector;
using std::ostringstream;


void
WorkunitInformation::create_table(MYSQL *conn) throw (string) {
    string query = "CREATE TABLE `tao_workunit_information` ("
                   "  `search_name` varchar(256) NOT NULL,"
                   "  `app_id` int (11) NOT NULL DEFAULT '0',"
                   "  `workunit_xml_filename` varchar(256) NOT NULL DEFAULT '',"
                   "  `result_xml_filename` varchar(256) NOT NULL DEFAULT '',"
                   "  `input_filenames` varchar(1024) NOT NULL DEFAULT '',"
                   "  `command_line_options` varchar(512) NOT NULL DEFAULT '',"
                   "  `extra_xml` varchar(512) NOT NULL DEFAULT '',"
                   "  PRIMARY KEY (`search_name`)"
                   ") Engine=InnoDB DEFAULT CHARSET=latin1";

    mysql_query(conn, query.c_str());

    if (mysql_errno(conn) != 0) {
        ostringstream ex_msg;
        ex_msg << "ERROR: could not create workunit information table using query: '" << query << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }
}

/**
 *  Get workunit information about a search from a database.
 */
WorkunitInformation::WorkunitInformation(MYSQL *conn, const string search_name) throw (string) {
    this->search_name = search_name;

    ostringstream query;
    query << "SELECT search_name, app_id, workunit_xml_filename, result_xml_filename, input_filenames, command_line_options, extra_xml FROM tao_workunit_information"
          << " WHERE search_name = '" << search_name << "'";

    mysql_query(conn, query.str().c_str());

    MYSQL_RES *result = mysql_store_result(conn);
    if (mysql_errno(conn) == 0) {
        MYSQL_ROW row = mysql_fetch_row(result);

        if (row == NULL) {
            ostringstream ex_msg;
            ex_msg << "ERROR: could not row from workunit query: '" << query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
            throw ex_msg.str();
        }

//        cerr << "query: " << query.str() << endl;
//        cerr << "workunit info row: " << row[1] << ", " << row[2] << ", " << row[3] << ", " << row[4] << ", " << row[5] << ", " << row[6] << endl;

        app_id = atoi(row[1]);
        workunit_xml_filename = row[2];
        result_xml_filename = row[3];
        string_to_vector<string>(row[4], input_filenames);
        command_line_options = row[5];
        extra_xml = row[6];

    } else {
        ostringstream ex_msg;
        ex_msg << "ERROR: could not get workunit information from query: '" << query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }

    mysql_free_result(result);

}

/**
 *  Commit workunit information about a search to a database.
 */

WorkunitInformation::WorkunitInformation(MYSQL *conn,
                                         const string search_name, 
                                         const int app_id,
                                         const string workunit_xml_filename,
                                         const string result_xml_filename,
                                         const vector<string> &input_filenames,
                                         const string command_line_options,
                                         const string extra_xml
                                        ) throw (string) {
    this->search_name = search_name;

    ostringstream query;
    query << "INSERT INTO tao_workunit_information"
          << " SET "
          << "  search_name = '" << search_name << "'"
          << ", app_id = " << app_id
          << ", workunit_xml_filename = '" << workunit_xml_filename << "'"
          << ", result_xml_filename = '" << result_xml_filename << "'"
          << ", input_filenames = '" << vector_to_string<string>(input_filenames) << "'"
          << ", command_line_options = '" << command_line_options << "'"
          << ", extra_xml = '" << extra_xml << "'";

    mysql_query(conn, query.str().c_str());

    if (mysql_errno(conn) != 0) {
        ostringstream ex_msg;
        ex_msg << "ERROR: could not insert workunit information using query: '" << query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
        throw ex_msg.str();
    }
}

void
WorkunitInformation::print_to(ostream& stream) {
    stream << "[WorkunitInformation " << endl
           << "    search_name = " << search_name << endl
           << "    app_id = " << app_id << endl
           << "    workunit_xml_filename = '" << workunit_xml_filename << "'" << endl
           << "    result_xml_filename = '" << result_xml_filename << "'" << endl
           << "    input_filenames = '" << vector_to_string(input_filenames) << "'" << endl
           << "    command_line_options = '" << command_line_options << "'" << endl
           << "    extra_xml = '" << extra_xml << "'" << endl
           << "]" << endl;
}

ostream& operator<< (ostream& stream, WorkunitInformation &wu_info) {
    wu_info.print_to(stream);
    return stream;
}
