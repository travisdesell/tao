#ifndef TAO_MATRIX_H
#define TAO_MATRIX_H

#include <vector>
#include <string>

#include "stdint.h"

using std::vector;
using std::string;

vector< vector<double> > matrix_transpose(const vector< vector<double> > &m) throw (string);

vector< vector<double> > matrix_multiply(const vector< vector<double> > &m1, const vector< vector<double> > &m2) throw (string);

vector<double> matrix_vector_multiply(const vector< vector<double> > &m, const vector<double> &v) throw (string);


void LUP_decomposition(const vector<vector <double> > &A, vector <vector <double> > &LU, vector<uint32_t> &P) throw (string);

void LUP_solve(const vector< vector<double> > &LU, const vector<uint32_t> &P, const vector<double> &b, vector<double> &result) throw (string);

vector< vector<double> > matrix_invert(const vector< vector<double> > &m) throw (string);

#endif
