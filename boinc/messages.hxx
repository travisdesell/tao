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
