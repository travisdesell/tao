#ifndef TAO_BOINC_MESSAGES_H
#define TAO_BOINC_MESSAGES_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include "mysql.h"

#include "messages.hxx"

using namespace std;

void get_applications(MYSQL *conn, vector<string> &names, vector<int> &ids);

void print_applications(MYSQL *conn);

void print_searches(MYSQL *conn, int app_id);

#endif
