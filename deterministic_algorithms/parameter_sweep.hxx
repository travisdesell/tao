#ifndef TAO_PARAMETER_SWEEP_H
#define TAO_PARAMETER_SWEEP_H

#include <vector>

void parameter_sweep(const std::vector<double> &min_bound, const std::vector<double> &max_bound, const std::vector<double> &step_size, double (*objective_function)(const std::vector<double> &));

#endif
