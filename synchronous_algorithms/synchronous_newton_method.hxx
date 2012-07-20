#ifndef TAO_NEWTON_METHOD_H
#define TAO_NEWTON_METHOD_H

#include <string>
#include <vector>

using namespace std;

void synchronous_newton_method(vector<string> arguments, double (*objective_function)(const std::vector<double> &));

void synchronous_newton_method(vector<string> arguments, double (*objective_function)(const std::vector<double> &), const vector<double> &starting_point, const vector<double> &step_size);

void synchronous_newton_method(vector<string> arguments, double (*objective_function)(const std::vector<double> &), const vector<double> &min_bound, const vector<double> &max_bound, const vector<double> &starting_point, const vector<double> &step_size);


#endif
