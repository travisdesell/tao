#ifndef TAO_NEWTON_STEP_H
#define TAO_NEWTON_STEP_H

#include <vector>

using std::vector;

void newton_step(const vector< vector<double> > &hessian, const vector<double> &gradient, vector<double> &step) throw (string);

#endif
