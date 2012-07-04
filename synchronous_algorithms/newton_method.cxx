#include <vector>

#include "newton_method.hxx"

#include "matrix.hxx"

using std::vector;

void newton_step(const vector< vector<double> > &hessian, const vector<double> &gradient, vector<double> &step) {
	vector< vector<double> > inverse_hessian();

	matrix_invert(hessian, number_parameters, number_parameters, inverse_hessian);
	matrix_vector_multiply(inverse_hessian, gradient, step);
}
