#ifndef FPOPS_FROM_PARAMETERS_H
#define FPOPS_FROM_PARAMETERS_H

#include <vector>
#include <string>

using std::string;
using std::vector;

void calculate_fpops(const vector<double> &parameters, double &rsc_fpops_est, double &rsc_fpops_bound, string workunit_extra_xml);

#endif
