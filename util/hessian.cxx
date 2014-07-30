#include <iostream>
#include <vector>

#include "stdint.h"

#include "util/hessian.hxx"
#include "util/matrix.hxx"

//#include <boost/numeric/ublas/matrix.hpp>
//#include <boost/numeric/ublas/lu.hpp>
//#include <boost/numeric/ublas/io.hpp>

#include "vector_io.hxx"

using std::vector;
using std::cout;
using std::endl;

//using namespace boost::numeric::ublas; 

void get_hessian(double (*objective_function)(const std::vector<double> &), const vector<double> &point, const vector<double> &step, vector< vector<double> > &hessian) {
    vector<double> point_copy(point);

    double e1, e2, e3, e4;
    double pi, pj;

    for (uint32_t i = 0; i < point_copy.size(); i++) {
        for (uint32_t j = 0; j < point_copy.size(); j++) {
            pi = point_copy[i];
            pj = point_copy[j];

            if (i == j) {
                point_copy[i] = pi + step[i] + step[i];

                e1 = objective_function(point_copy);
//                cout << "evaluating point: " << vector_to_string(point_copy) << " = " << e1 << endl;
                point_copy[i] = pi;

                e2 = objective_function(point_copy);
                e3 = e2;
//                cout << "evaluating point: " << vector_to_string(point_copy) << " = " << e2 << " == " << e3 << endl;

                point_copy[i] = pi - (step[i] + step[i]); 
                e4 = objective_function(point_copy);
//                cout << "evaluating point: " << vector_to_string(point_copy) << " = " << e4 << endl;

            } else {
                point_copy[i] = pi + step[i];
                point_copy[j] = pj + step[j];
                e1 = objective_function(point_copy);
//                cout << "evaluating point: " << vector_to_string(point_copy) << " = " << e1 << endl;

                point_copy[i] = pi - step[i];
                e2 = objective_function(point_copy);
//                cout << "evaluating point: " << vector_to_string(point_copy) << " = " << e2 << endl;

                point_copy[i] = pi + step[i];
                point_copy[j] = pj - step[j];
                e3 = objective_function(point_copy);
//                cout << "evaluating point: " << vector_to_string(point_copy) << " = " << e3 << endl;

                point_copy[i] = pi - step[i];
                e4 = objective_function(point_copy);
//                cout << "evaluating point: " << vector_to_string(point_copy) << " = " << e4 << endl;
            }

            point_copy[i] = pi;
            point_copy[j] = pj;

            hessian[i][j] = (e1 - e3 - e2 + e4)/(4 * step[i] * step[j]);
            cout << "\t\thessian[" << i << "][" << j << "]: " << hessian[i][j] << " = (" << e1 << " - " << e3 << " - " << e2 << " + " << e4 << ") / (4 * " << step[i] << " * " << step[j] << ")" << endl;
        }
    }
}

/** Matrix inversion routine.
 *  Uses lu_factorize and lu_substitute in uBLAS to invert a matrix.
 */
/*
template<class T>
bool InvertMatrix(const matrix<T>& input, matrix<T>& inverse) {
    typedef permutation_matrix<std::size_t> pmatrix;

    // create a working copy of the input
    matrix<T> A(input);

    // create a permutation matrix for the LU-factorization
    pmatrix pm(A.size1());

    // perform LU-factorization
    int res = lu_factorize(A, pm);
    if (res != 0) return false;

    // create identity matrix of "inverse"
    inverse.assign(identity_matrix<T> (A.size1()));

    // backsubstitute to get the inverse
    lu_substitute(A, pm, inverse);

    return true;
}

void randomized_hessian(const vector< vector<double> > &points, const vector<double> &fitness, const vector<double> &center, vector< vector< double> > &hessian, vector<double> &gradient) {
    uint32_t number_parameters = center.size();

    X LENGTH IS NOT RIGHT
    uint32_t x_length = 1 + number_parameters + number_parameters + (number_parameters * number_parameters);

    matrix<double> X(points.size(), x_length);     //create a matrix: number of points by x_length
    matrix<double> Y(points.size(), 1);

    for (uint32_t i = 0; i < points.size()) {
        Y(i, 0) = fitness[i];

        X(i, 0) = 1.0;
        for (uint32_t j = 0; j < number_parameters; j++) {
            X(i, 1 + j) = point[i][j];
            X(i, 1 + number_parameters + j) = 0.5 * points[i][j] * points[i][j];
        }

        current = 0;
        for (uint32_t j = 0; j < number_parameters; j++) {
            for (uint32_t k = 0; k < number_parameters; k++) {
                X(i, 1 + number_parameters + number_parameters + current) = points[i][j] * points[i][k];
                current++;
            }
        }
    }

    // W = (X^T * X)^-1 * X^T * Y
    matrix<double> X_transpose = trans(X);

    matrix<double> XTX_inverted(points.size(), points.size());
    InvertMatrix(X_transpose * X, XTX_inverted);

    matrix<double> W = XTX_inverted * X_transpose * Y;

    for (i = 0; i < number_parameters; i++) {
        gradient[i] = W(1 + i, 0);
        hessian[i][i] = W(1 + number_parameters + i, 0);

        current = 0;
        for (j = i; j < number_parameters; j++) {
            for (k = j+1; k < number_parameters; k++) {
                hessian[j][k] = W(1 + number_parameters + number_parameters + current, 0);
                hessian[k][j] = W(1 + number_parameters + number_parameters + current, 0);
                current++;
            }
        }
    }
}
*/

/*
 *  OLD C VERSION AS FOLLOWS.
 *  TODO: compare vs boost version above
 */
void randomized_hessian(const vector< vector<double> > &actual_points, const vector<double> &center, const vector<double> &fitness, vector< vector<double> > &hessian, vector<double> &gradient) throw (string) {
    vector< vector<double> > points( actual_points );
    for (uint32_t i = 0; i < points.size(); i++) {
        for (uint32_t j = 0; j < points[i].size(); j++) {
            points[i][j] = center[j] - points[i][j];
        }
    }

    uint32_t number_parameters = points[0].size();
    /********
     *	X = [1, x1, ... xn, 0.5*x1^2, ... 0.5*xn^2, x1*x2, ..., x1*xn, x2*x3, ..., x2*xn, ...]
     ********/
    uint32_t x_len = 1 + number_parameters + number_parameters;
    for (uint32_t i = number_parameters - 1; i > 0; i--) x_len += i;

    vector< vector<double> > Y(points.size(), vector<double>(1, 0.0));
    vector< vector<double> > X(points.size(), vector<double>(x_len, 0.0));

    for (uint32_t i = 0; i < points.size(); i++) {
        Y[i][0] = fitness[i];

        X[i][0] = 1;
        for (uint32_t j = 0; j < number_parameters; j++) {
            X[i][1+j] = points[i][j];
            X[i][1+number_parameters+j] = 0.5 * points[i][j] * points[i][j];
        }
        uint32_t current = 0;
        for (uint32_t j = 0; j < number_parameters; j++) {
            for (uint32_t k = j+1; k < number_parameters; k++) {
                X[i][1+number_parameters+number_parameters+current] = points[i][j] * points[i][k];
                current++;
            }
        }
    }

    vector< vector<double> > X_transpose    = matrix_transpose(X);
    vector< vector<double> > X2             = matrix_multiply(X_transpose, X);
    vector< vector<double> > X_inverse      = matrix_invert(X2);
    vector< vector<double> > X3             = matrix_multiply(X_inverse, X_transpose);
    vector< vector<double> > W              = matrix_multiply(X3, Y);

    gradient.resize(number_parameters);
    hessian.resize(number_parameters, vector<double>(number_parameters));

    for (uint32_t i = 0; i < number_parameters; i++) {
        gradient[i] = W[1+i][0];
        hessian[i][i] = W[1 + number_parameters + i][0];

        uint32_t current = 0;
        for (uint32_t j = i; j < number_parameters; j++) {
            for (uint32_t k = j+1; k < number_parameters; k++) {
                hessian[j][k] = W[1 + number_parameters + number_parameters + current][0];
                hessian[k][j] = W[1 + number_parameters + number_parameters + current][0];
                current++;
            }
        }
    }
}
