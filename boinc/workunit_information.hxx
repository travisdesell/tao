#ifndef TAO_WORKUNIT_INFORMATION
#define TAO_WORKUNIT_INFORMATION

#include <string>
#include <vector>


using std::string;
using std::vector;

class WorkunitInformation {
    protected:
        int search_id;

    public:
        string workunit_xml_filename;
        string result_xml_filename;
        vector<string> input_filenames;
        string command_line_options;
        string extra_xml;

        WorkunitInformation(MYSQL *conn,
                            const int search_id
                           );

        WorkunitInformation(MYSQL *conn,
                            const int search_id,
                            const string workunit_xml_filename,
                            const string result_xml_filename,
                            const vector<string> &input_filenames,
                            const string command_line_options,
                            const string extra_xml
                           );
};


#endif
