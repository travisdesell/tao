#ifndef TAO_MATRIX_H
#define TAO_MATRIX_H

#include <vector>
#include <string>

#include "stdint.h"

using std::vector;
using std::string;

vector< vector<double> > matrix_transpose(const vector< vector<double> > &m) throw (const char*);

vector< vector<double> > matrix_multiply(const vector< vector<double> > &m1, const vector< vector<double> > &m2) throw (const char*);

vector<double> matrix_vector_multiply(const vector< vector<double> > &m, const vector<double> &v) throw (const char*);


void LUP_decomposition(const vector<vector <double> > &A, vector <vector <double> > &LU, vector<uint32_t> &P) throw (const char*);

void LUP_solve(const vector< vector<double> > &LU, const vector<uint32_t> &P, const vector<double> &b, vector<double> &result) throw (const char*);

vector< vector<double> > matrix_invert(const vector< vector<double> > &m) throw (const char*);

#endif
