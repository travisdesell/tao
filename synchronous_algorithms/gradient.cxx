#include <vector>
#include <iostream>

#include "stdint.h"

#include "gradient.hxx"

using std::vector;
using std::cout;
using std::endl;

void get_gradient(double (*objective_function)(const std::vector<double> &), const vector<double> &point, const vector<double> &step, vector <double> &gradient) {
    vector <double> point_copy(point);

	double e1, e2, p_i;

	for (uint32_t i = 0; i < point_copy.size(); i++) {
		p_i = point_copy[i];
		point_copy[i] = p_i + step[i];
		e1 = objective_function(point_copy);
		point_copy[i] = p_i - step[i];
		e2 = objective_function(point_copy);
		point_copy[i] = p_i;

		gradient[i] = (e1 - e2)/(step[i] + step[i]);
//        cout << "\t\tgradient[" << i << "] " << gradient[i] << ", (" << e1 << " - " << e2 << ")/(2.0 * " << step[i] << ")" << endl;
	}
}

bool gradient_below_threshold(const vector<double> &gradient, const vector<double> &threshold) {
	for (uint32_t i = 0; i < gradient.size(); i++) {
		if (gradient[i] > threshold[i] || gradient[i] < -threshold[i]) return false;
	}
	return true;
}
