#include "config.h"
#include <vector>
#include <cstdlib>
#include <string>

#include "boinc_db.h"

using namespace std;

//returns 0 on sucess
int assimilate_handler(WORKUNIT& wu, vector<RESULT>& /*results*/, RESULT& canonical_result) {
    //Don't need to do anything, when the result is validated it gets inserted into the database directly
    return 0;
}
