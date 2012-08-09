#include <cmath>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>

#include "stdint.h"

#include "util/matrix.hxx"

#include "undvc_common/vector_io.hxx"

using std::cout;
using std::endl;
using std::string;
using std::ostringstream;
using std::vector;

#define swap(type, i, j) {type t = i; i = j; j = t;}

vector< vector<double> > identity_matrix(uint32_t r) {
    vector< vector<double> > m(r, vector<double>(r, 0.0));

	for (uint32_t i = 0; i < r; i++) m[i][i] = 1.0;

    return m;
}

/**
 * 	Matrix Transpose Code
 */
vector< vector<double> > matrix_transpose(const vector< vector<double> > &m) throw (string) {
    //does not need to be square
//    if (m.size() != m[0].size()) {
//        ostringstream err_msg;
//        err_msg << "can only transpose square matrices, this one had rows (" << m.size() << ") != columns (" << m[0].size() << ")";
//        throw err_msg.str();
//    }

//    cout << "transposing: " << m.size() << " x " << m[0].size() << endl;
//    cout << "transposing: " << vector_2d_to_string(m) << endl;

    vector< vector<double> > result(m[0].size(), vector<double>(m.size()));

	for (uint32_t i = 0; i < m[0].size(); i++) {
		for (uint32_t j = 0; j < m.size(); j++) {
			result[i][j] = m[j][i];
		}
	}

//    cout << "into:        " << result.size() << " x " << result[0].size() << endl;
//    cout << "into:        " << vector_2d_to_string(result) << endl;
    return result;
}

/**
 * 	Matrix Multiplication Code
 */
vector< vector<double> > matrix_multiply(const vector< vector<double> > &m1, const vector< vector<double> > &m2) throw (string) {
	if (m1[0].size() != m2.size()) {
        ostringstream err_msg;
        err_msg << "matrix multiply error, columns of first matrix[" << m1[0].size() << " do not match rows of the second matrix [" << m2.size() << "]" << endl;
        throw err_msg.str();
	}

//    cout << "multiplying: " << m1.size() << " x " << m1[0].size() << endl;
//    cout << "by:          " << m2.size() << " x " << m2[0].size() << endl;

    vector< vector<double> > result(m1.size(), vector<double>(m2[0].size()));
//    cout << "into:        " << result.size() << " x " << result[0].size() << endl;

	for (uint32_t i = 0; i < m1.size(); i++) {
		for (uint32_t j = 0; j < m2[0].size(); j++) {
//            cout << "setting " << i << ", " << j << endl;

			result[i][j] = 0;
			for (uint32_t k = 0; k < m1[0].size(); k++) {
				result[i][j] += m1[i][k] * m2[k][j];
			}
		}
	}
    return result;
}

//a matrix times a vector is a vector (result is rows of first by columns of second)
//columns of the matrix must == rows of the vector 
vector<double> matrix_vector_multiply(const vector< vector<double> > &m, const vector<double> &v) throw (string) {
	if (m[0].size() != v.size()) {
        ostringstream err_msg;
        err_msg << "matrix vector multiply error, columns of matrix[" << m[0].size() << " must match length of the vector [" << v.size() << "]" << endl;
        throw err_msg.str();
    }

    vector<double> result(m.size());
	for (uint32_t j = 0; j < m.size(); j++) {
		result[j] = 0;
		for (uint32_t k = 0; k < m[0].size(); k++) {
			result[j] += m[j][k] * v[k];
		}
	}

    return result;
}

/**
 * 	Matrix Inversion and LUP decomposition
 */
void LUP_decomposition(const vector<vector <double> > &A, vector <vector <double> > &LU, vector<uint32_t> &P) throw (string) {
	double p, divisor;

    uint32_t length = A.size();

	uint32_t k_prime = 0;

    LU.assign(A.begin(), A.end());

    P.resize(length, 0.0);
	for (uint32_t i = 0; i < length; i++) P[i] = i;

	for (uint32_t k = 0; k < length; k++) {
		p = 0;
		for (uint32_t i = k; i < length; i++) {
			if (fabs(LU[i][k]) > p) {
				p = fabs(LU[i][k]);
				k_prime = i;
			}
		}

		//This is a singular matrix.
		if (p == 0) throw "Singular matrix passed to LUP_decomposition";

		swap(uint32_t, P[k], P[k_prime]);
		for (uint32_t i = 0; i < length; i++) {
			swap(double, LU[k][i], LU[k_prime][i]);
		}

		divisor = LU[k][k];
		for (uint32_t i = k+1; i < length; i++) {
			LU[i][k] = LU[i][k]/divisor;
			for (uint32_t j = k+1; j < length; j++) {
				LU[i][j] = LU[i][j] - LU[i][k]*LU[k][j];
			}
		}
	}
}

void LUP_solve(const vector< vector<double> > &LU, const vector<uint32_t> &P, const vector<double> &b, vector<double> &result) throw (string) {
    int32_t length = LU.size();

    vector<double> y(length, 0.0);

	for (int32_t i = 0; i < length; i++) {
		y[i] = b[P[i]];
		for (int32_t j = 0; j < i; j++) {
			y[i] -= LU[i][j] * y[j];
		}
	}

    result.assign(y.begin(), y.end());

	for (int32_t i = length - 1; i >= 0; i--) {
//		result[i] = y[i];   -- handled in assign
		for (int32_t j = i+1; j < length; j++) {
			result[i] -= LU[i][j] * result[j];
		}
		result[i] /= LU[i][i];
	}
}

vector< vector<double> > matrix_invert(const vector< vector<double> > &initial) throw (string) {
    vector< vector<double> > LU;
    vector<uint32_t> p;
	LUP_decomposition(initial, LU, p);

    vector< vector<double> > identity = identity_matrix(initial.size());

    vector< vector<double> > result(initial[0].size());

	for (uint32_t i = 0; i < initial[0].size(); i++) {
		LUP_solve(LU, p, identity[i], result[i]);
	}

	return matrix_transpose(result);
}

#ifdef MATRIX_MUL_TEST

int main(int argc, char **argv) {
    vector< vector<double> > a(2, vector<double>(3, 0.0));
    vector< vector<double> > b(3, vector<double>(2, 0.0));
	//1 0 2
	//-1 3 1
	//
	//3 1
	//2 1
	//1 0
	a[0][0] = 1;
	a[0][1] = 0;
	a[0][2] = 2;
	a[1][0] = -1;
	a[1][1] = 3;
	a[1][2] = 1;

	b[0][0] = 3;
	b[0][1] = 1;
	b[1][0] = 2;
	b[1][1] = 1;
	b[2][0] = 1;
	b[2][1] = 0;

    cout << "a:" << vector_2d_to_string(a) << endl; 
    cout << "b:" << vector_2d_to_string(b) << endl; 

    vector< vector<double> > c = matrix_multiply(a, b);

    cout << "a * b = c: " << vector_2d_to_string(c) << endl;
    cout << "should be:" << endl;
    cout << " 5 1" << endl;
    cout << " 4 2" << endl;

	c = matrix_multiply(b, a);

    cout << "b * a = c2: " << vector_2d_to_string(c) << endl;
    cout << "should be: " << endl;

	a[0][0] = 1;
	a[0][1] = -1;
	a[0][2] = 2;
	a[1][0] = 0;
	a[1][1] = -3;
	a[1][2] = 1;

    vector<double> v(3, 0.0);
	v[0] = 2;
	v[1] = 1;
	v[2] = 0;

    cout << endl << endl;
    cout << "a: " << vector_2d_to_string(a) << endl;
    cout << "v: " << vector_to_string(v) << endl;

    vector<double> w = matrix_vector_multiply(a, v);
    cout << "a * v = w: " << vector_to_string(w) << endl;
    cout << "should be: 1 -3" << endl;
}
#endif


#ifdef MATRIX_INVERSE_TEST
int main(int argc, char **argv) {
    vector< vector<double> > matrix(4, vector<double>(4, 0.0));
	matrix[0][0] = 2;
	matrix[0][1] = 0;
	matrix[0][2] = 2;
	matrix[0][3] = 0.6;
	matrix[1][0] = 3;
	matrix[1][1] = 3;
	matrix[1][2] = 4;
	matrix[1][3] = -2;
	matrix[2][0] = 5;
	matrix[2][1] = 5;
	matrix[2][2] = 4;
	matrix[2][3] = 2;
	matrix[3][0] = -1;
	matrix[3][1] = -2;
	matrix[3][2] = 3.4;
	matrix[3][3] = -1;

    cout.precision(10);

    cout << "matrix:" << vector_2d_to_string(matrix) << endl;

    vector< vector<double> > inverse = matrix_invert(matrix);
    cout << "inverse:" << vector_2d_to_string(inverse) << endl;

    vector< vector<double> > result = matrix_multiply(matrix, inverse);
    cout << "result:" << vector_2d_to_string(result) << endl;

    cout << endl << endl;

	matrix[0][0] = 1;
	matrix[0][1] = 3;
	matrix[0][2] = 3;
	matrix[1][0] = 1;
	matrix[1][1] = 4;
	matrix[1][2] = 3;
	matrix[2][0] = 1;
	matrix[2][1] = 3;
	matrix[2][2] = 4;
	/*
	matrix[0][0] = 1;
	matrix[0][1] = 2;
	matrix[0][2] = 0;
	matrix[1][0] = 3;
	matrix[1][1] = 4;
	matrix[1][2] = 4;
	matrix[2][0] = 5;
	matrix[2][1] = 6;
	matrix[2][2] = 3;
	*/

    vector<double> b(3, 0.0);
    b[0] = 3;
    b[1] = 7;
    b[2] = 8;

    cout << "matrix:" << vector_2d_to_string(matrix) << endl;
	inverse = matrix_invert(matrix);

    vector<uint32_t> p;
    vector< vector<double> > LU;
	LUP_decomposition(matrix, LU, p);

    cout << "LU: " << vector_2d_to_string(LU) << endl;
    cout << "p: " << vector_to_string(p) << endl;

    vector<double> result2;
	LUP_solve(LU, p, b, result2);

    cout << "result: " << vector_to_string(result2) << endl;
	
    cout << "inverse:" << vector_2d_to_string(inverse) << endl;
	result = matrix_multiply(matrix, inverse);
    cout << "result:" << vector_2d_to_string(result) << endl;
}
#endif

