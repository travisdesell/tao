#ifndef TAO_HESSIAN_H
#define TAO_HESSIAN_H

#include <vector>

using std::vector;

void get_hessian(double (*objective_function)(const std::vector<double> &), const vector<double> &point, const vector<double> &step, vector< vector<double> > &hessian);

void randomized_hessian(const vector< vector<double> > &points, const vector<double> &fitness, const vector<double> &center, vector< vector< double> > &hessian, vector<double> &gradient);

#endif
