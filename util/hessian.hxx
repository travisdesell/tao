#ifndef TAO_HESSIAN_H
#define TAO_HESSIAN_H

#include <vector>
#include <string>

using std::vector;
using std::string;

void get_hessian(double (*objective_function)(const std::vector<double> &), const vector<double> &point, const vector<double> &step, vector< vector<double> > &hessian);

void randomized_hessian(const vector< vector<double> > &actual_points, const vector<double> &center, const vector<double> &fitness, vector< vector<double> > &hessian, vector<double> &gradient) throw (string);

#endif
