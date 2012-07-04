#ifndef TAO_GRADIENT_H
#define TAO_GRADIENT_H

#include <vector>

using std::vector;

void get_gradient(double (*objective_function)(const std::vector<double> &), const vector<double> &point, const vector<double> &step, vector <double> &gradient);

bool gradient_below_threshold(const vector<double> &gradient, const vector<double> &threshold);


#endif
