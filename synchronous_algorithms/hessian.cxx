#include <iostream>
#include <vector>

#include "hessian.hxx"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>

using std::vector;
using std::cout;
using std::endl;

using namespace boost::numeric::ublas; 

void get_hessian(double (*objective_function)(const std::vector<double> &), const vector<double> &point, const vector<double> &step, vector< vector<double> > &hessian) {
    int i, j;
    double e1, e2, e3, e4;
    double pi, pj;

    for (uint32_t i = 0; i < point.size(); i++) {
        for (uint32_t j = 0; j < point.size(); j++) {
            pi = point[i];
            pj = point[j];

            if (i == j) {
                point[i] = pi + step[i] + step[i];

                e1 = objective_function(point);
                point[i] = pi;

                e2 = objective_function(point);
                e3 = e2;

                point[i] = pi - (step[i] + step[i]); 
                e4 = objective_function(point);

            } else {
                point[i] = pi + step[i];
                point[j] = pj + step[j];
                e1 = objective_function(point);

                point[i] = pi - step[i];
                e2 = objective_function(point);

                point[i] = pi + step[i];
                point[j] = pj - step[j];
                e3 = objective_function(point);

                point[i] = pi - step[i];
                e4 = objective_function(point);
            }

            point[i] = pi;
            point[j] = pj;

            hessian[i][j] = (e1 - e3 - e2 + e4)/(4 * step[i] * step[j]);
            cout << "\t\thessian[" << i << "][" << j << "]: " << hessian[i][j] << endl;
        }
    }
}

/** Matrix inversion routine.
 *  Uses lu_factorize and lu_substitute in uBLAS to invert a matrix.
 */
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

/*
 *  OLD C VERSION AS FOLLOWS (maybe want to test)
 *
void randomized_hessian(double** actual_points, double* center, double* fitness, int number_points, int number_parameters, double*** hessian, double** gradient) {
    double** Y;
    double** X;
    double** X2;
    double** X3;
    double** X_transpose;
    double** X_inverse;
    double** W;
    double** points;
    int x_len, i, j, k, current;

    points = (double**)malloc(sizeof(double*) * number_points);
    for (i = 0; i < number_points; i++) {
        points[i] = (double*)malloc(sizeof(double) * number_parameters);
        for (j = 0; j < number_parameters; j++) {
            points[i][j] = center[j] - actual_points[i][j];
        }
    }


    /********
     *	X = [1, x1, ... xn, 0.5*x1^2, ... 0.5*xn^2, x1*x2, ..., x1*xn, x2*x3, ..., x2*xn, ...]
     ********/ /*
    x_len = 1 + number_parameters + number_parameters;
    for (i = number_parameters - 1; i > 0; i--) x_len += i;


    Y = (double**)malloc(sizeof(double*) * number_points);
    X = (double**)malloc(sizeof(double) * number_points);
    for (i = 0; i < number_points; i++) {
        Y[i] = (double*)malloc(sizeof(double) * 1);
        Y[i][0] = fitness[i];
        X[i] = (double*)malloc(sizeof(double) * x_len);
        X[i][0] = 1;
        for (j = 0; j < number_parameters; j++) {
            X[i][1+j] = points[i][j];
            X[i][1+number_parameters+j] = 0.5 * points[i][j] * points[i][j];
        }
        current = 0;
        for (j = 0; j < number_parameters; j++) {
            for (k = j+1; k < number_parameters; k++) {
                X[i][1+number_parameters+number_parameters+current] = points[i][j] * points[i][k];
                current++;
            }
        }
    }

    matrix_transpose__alloc(X, number_points, x_len, &X_transpose);
    matrix_multiply__alloc(X_transpose, x_len, number_points, X, number_points, x_len, &X2);
    matrix_invert__alloc(X2, x_len, x_len, &X_inverse);
    matrix_multiply__alloc(X_inverse, x_len, x_len, X_transpose, x_len, number_points, &X3);
    matrix_multiply__alloc(X3, x_len, number_points, Y, number_points, 1, &W);


    (*gradient) = (double*)malloc(sizeof(double) * number_parameters);
    (*hessian) = (double**)malloc(sizeof(double*) * number_parameters);
    for (i = 0; i < number_parameters; i++) (*hessian)[i] = (double*)malloc(sizeof(double) * number_parameters);

    for (i = 0; i < number_parameters; i++) {
        (*gradient)[i] = W[1+i][0];
        (*hessian)[i][i] = W[1 + number_parameters + i][0];

        current = 0;
        for (j = i; j < number_parameters; j++) {
            for (k = j+1; k < number_parameters; k++) {
                (*hessian)[j][k] = W[1 + number_parameters + number_parameters + current][0];
                (*hessian)[k][j] = W[1 + number_parameters + number_parameters + current][0];
                current++;
            }
        }
    }

    /********
     *	Y = number_points by 1
     *	X = number_points by x_len
     *	X2 = x_len by x_len
     *	X3 = x_len by number_points
     *	X_transpose = x_len by number_points
     *	X_inverse = x_len by x_len
     *	W = x_len by 1
     ********/ /*
    for (i = 0; i < number_points; i++) {
        free(Y[i]);
        free(X[i]);
        free(points[i]);
    }
    for (i = 0; i < x_len; i++) {
        free(X2[i]);
        free(X3[i]);
        free(X_transpose[i]);
        free(X_inverse[i]);
        free(W[i]);
    }
    free(points);
    free(Y);
    free(X);
    free(X2);
    free(X3);
    free(X_transpose);
    free(X_inverse);
    free(W);
} */
