#include "mysql.h"
#include "workunit_information.hxx"


/**
 *  Get workunit information about a search from a database.
 */
WorkunitInformation::WorkunitInformation(MYSQL *conn, int search_id) :: throw (string) {
    this->search_id = search_id;

    mysql_query(conn, "SELECT * FROM tao_workunit_information");

    MYSQL_RES *result = mysql_store_result(conn);
    if (mysql_errno(conn) == 0) {
        MYSQL_ROW row = mysql_fetch_row(result);

        if (row == NULL) {
            ostringstream ex_msg;
            ex_msg << "ERROR: could not row from workunit query: '" << query.str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << __FILE__ << ":" << __LINE__;
            throw ex_msg.str();
        }

        workunit_xml_filename = row[1];
        result_xml_filename = row[2];
        input_filenames = string_to_vector<string>(row[3]);
        command_line_options = row[4];
        extra_xml = row[5];

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
                                         const int search_id,
                                         const string workunit_xml_filename,
                                         const string result_xml_filename,
                                         const vector<string> &input_filenames,
                                         const string command_line_options,
                                         const string extra_xml
                                        ) throw (string) {
    this->search_id = search_id;

    ostringstream query;
    query << "INSERT INTO tao_workunit_information"
          << " SET "
          << "  search_id = " << search_id <<
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
