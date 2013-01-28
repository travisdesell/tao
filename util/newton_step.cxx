#include <vector>

#include "util/newton_step.hxx"
#include "util/matrix.hxx"

using std::vector;

void newton_step(const vector< vector<double> > &hessian, const vector<double> &gradient, vector<double> &step) throw (string) { 
	vector< vector<double> > inverse_hessian = matrix_invert(hessian);
	step = matrix_vector_multiply(inverse_hessian, gradient);
}
