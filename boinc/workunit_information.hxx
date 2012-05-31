#ifndef TAO_WORKUNIT_INFORMATION
#define TAO_WORKUNIT_INFORMATION

#include <string>
#include <vector>

using std::string;
using std::vector;

class WorkunitInformation {
    protected:
        int search_id;
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
                            const int search_id
                           ) throw (string);

        WorkunitInformation(MYSQL *conn,
                            const int search_id,
                            const int app_id,
                            const string workunit_xml_filename,
                            const string result_xml_filename,
                            const vector<string> &input_filenames,
                            const string command_line_options,
                            const string extra_xml
                           ) throw (string);

        static void create_table(MYSQL *conn) throw (string);
};


#endif
