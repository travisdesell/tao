/*
Copyright 2012, 2009 Travis Desell and the University of North Dakota.

This file is part of the Toolkit for Asynchronous Optimization (TAO).

TAO is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TAO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TAO.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef FPOPS_FROM_PARAMETERS_H
#define FPOPS_FROM_PARAMETERS_H

#include <vector>
#include <string>

using std::string;
using std::vector;

void calculate_fpops(const vector<double> &parameters, double &rsc_fpops_est, double &rsc_fpops_bound, string workunit_extra_xml);

#endif
