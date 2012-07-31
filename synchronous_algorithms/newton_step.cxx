#include <vector>

#include "synchronous_algorithms/newton_step.hxx"
#include "util/matrix.hxx"

using std::vector;

void newton_step(const vector< vector<double> > &hessian, const vector<double> &gradient, vector<double> &step) throw (const char*) { 
	vector< vector<double> > inverse_hessian = matrix_invert(hessian);
	step = matrix_vector_multiply(inverse_hessian, gradient);
}
