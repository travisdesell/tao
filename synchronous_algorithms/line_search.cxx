#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#include <string>
#include <sstream>

#ifdef _MSC_VER
#include <float.h> //for _isnan and _isinf on windows
#define isnan(x) _isnan(x)
#define isinf(x) !_finite(x)
#endif

#include "stdint.h"

#include "synchronous_algorithms/gradient.hxx"
#include "synchronous_algorithms/line_search.hxx"
//#include "synchronous_algorithms/regression.hxx"

#include "undvc_common/arguments.hxx"
#include "undvc_common/vector_io.hxx"

using std::string;
using std::ostringstream;
using std::vector;
using std::cout;
using std::endl;


LineSearch::LineSearch(double (*objective_function)(const vector<double> &), vector<string> arguments) {
    this->objective_function = objective_function;

    if (!get_argument(arguments, "--loop1_max", false, LOOP1_MAX)) {
        cerr << "Argument '--loop1_max' not found, using default of 300 maximum iterations for loop 1 of the line search." << endl;
        LOOP1_MAX = 300;
    }

    if (!get_argument(arguments, "--loop2_max", false, LOOP2_MAX)) {
        cerr << "Argument '--loop2_max' not found, using default of 300 maximum iterations for loop 2 of the line search." << endl;
        LOOP2_MAX = 300;
    }

    if (!get_argument(arguments, "--nquad", false, NQUAD)) {
        cerr << "Argument '--nquad <i>' not found, using default of 4 iterations for loop 3 of the line search." << endl;

        NQUAD = 4;
    }

    if (!get_argument(arguments, "--tol", false, tol)) {
        cerr << "Argument '--tol <d>' not found, using default of 1e-6 for tolerance of dstar in loop 3 of the line search." << endl;
        tol = 1e-6;
    }

    threshold_specified = get_argument_vector(arguments, "--min_threshold", false, min_threshold);

    if (!threshold_specified) {
        cerr << "Argument '--min_threshold <d1, d2 .. dn>' not found. line search will not quit if the input direction is very small." << endl;
    }
}

LineSearch::LineSearch(double (*objective_function)(const vector<double> &), const double tol, const uint32_t LOOP1_MAX, const uint32_t LOOP2_MAX, const uint32_t NQUAD) {
    this->objective_function = objective_function;
    this->tol = tol;
    this->LOOP1_MAX = LOOP1_MAX;
    this->LOOP2_MAX = LOOP2_MAX;
    this->NQUAD = NQUAD;
}

LineSearch::~LineSearch() {
}

string LineSearch::get_status() {
    return status;
}

double LineSearch::evaluate_step(const vector<double> &point, const double step, const vector<double> &direction, vector<double> &current_point) {
    for (uint32_t i = 0; i < point.size(); i++) {
        current_point[i] = point[i] + (step * direction[i]);
    }
    return objective_function(current_point);
}

void LineSearch::line_search(const vector<double> &point, double initial_fitness, const vector<double> &direction, vector <double> &new_point, double &new_fitness) throw (string) {
    if (threshold_specified && gradient_below_threshold(direction, min_threshold)) {
        cerr << "\tDirection dropped below threshold:" << endl;
        cerr << "\tdirection:  " << vector_to_string(direction) << endl;
        cerr << "\tthreshold: " << vector_to_string(min_threshold) << endl;

        status = "direction dropped below threshold";
        throw status;
    }

    uint32_t evaluations_done;
    double d1, d2, d3, dstar;
    double a, b, c, top, bottom;

    status = "init";

    vector<double> current_point(point);

    cout << "\tline search starting at fitness: " << initial_fitness << endl;
    cout << "\tinitial_point: " << vector_to_string(current_point) << endl;

    /********
        *    Find f1 < f2
     ********/
    double step = 1.0;
    double f1 = initial_fitness;
    // f2 = evaluate( point + (direction * step) );
    double f2 = evaluate_step(point, step, direction, current_point);
    evaluations_done = 1;

    cout << "\t\tloop 1, evaluations: " << evaluations_done << ", step: " << step << ", fitness: " << f2 << endl;

    if (f1 > f2) {
        double temp;
        d1 = 1.0;
        d2 = 0;
        temp = f1;
        f1 = f2;
        f2 = temp;
        d3 = -1.0;
    } else {
        d1 = 0.0;
        d2 = 1.0;
        d3 = 2.0;
    }

    /********
        *    Find f1 < f2 > f3
     ********/
    double jump = 2.0;
    // f3 = evaluate( point + (d3 * step * direction) );
    double f3 = evaluate_step(point, d3 * step, direction, current_point);
    evaluations_done++;

    cout << "\t\tloop 2, evaluations: " << evaluations_done << ", step: " << (d3 * step) << ", fitness: " << f3 << endl;

    uint32_t eval_count = 0;
    while (f3 >= (f2 + tol) && !isnan(f1) && !isnan(f2) && !isnan(f3) && eval_count < LOOP2_MAX) {
        d1 = d2;
        f1 = f2;
        d2 = d3;
        f2 = f3;
        d3 = jump * d3;

        // f3 = evaluate( point + (d3 * step * direction) );
        f3 = evaluate_step(point, d3 * step, direction, current_point);
        evaluations_done++;
        eval_count++;

        cout << "\t\tloop 2, evaluations: " << evaluations_done << ", step: " << (d3 * step) << ", fitness: " << f3 << endl;
    }

    if (isnan(f1)) { status = "f1 was NAN in loop 2"; throw status; }
    if (isnan(f2)) { status = "f2 was NAN in loop 2"; throw status; }
    if (isnan(f3)) { status = "f3 was NAN in loop 2"; throw status; }

    if (eval_count >= LOOP2_MAX) {
        ostringstream ex_msg;
        ex_msg << "loop 2 reached maximum evaluation count: " << eval_count;
        status = ex_msg.str();
        throw status;
    }


    /********
        *    Find minimum
     ********/
    cout << "\t\tNeed d1 < d2 < d3" << endl;
    cout << "\t\t\td1:    " << d1 << ", f1: " << f1 << endl;
    cout << "\t\t\td2:    " << d2 << ", f2: " << f2 << endl;
    cout << "\t\t\td3:    " << d3 << ", f3: " << f3 << endl;

    if (d1 < d2 && d2 < d3) {
        //Do nothing, this is the order we want things in.
    } else if (d1 > d2 && d2 > d3) {
        cout << "\t\tswapping d1, d3 (and f1, f3), so d1, d2, d3 are in the correct order." << endl;
        //Swap d1, d3 (and f1, f3) so they are in order
        double temp;
        temp = d3;
        d3 = d1;
        d1 = temp;

        temp = f3;
        f3 = f1;
        f1 = temp;
    } else {
        cerr <<  "ERROR: before 3rd phase of line search, d1, d2 and d3 were not in order." << endl; //This should never happen
        exit(1);
    }
    cout << "\t\tShould be d1 < d2 < d3:" << endl;
    cout << "\t\t\td1:    " << d1 << ", f1: " << f1 << endl;
    cout << "\t\t\td2:    " << d2 << ", f2: " << f2 << endl;
    cout << "\t\t\td3:    " << d3 << ", f3: " << f3 << endl;

    double fs = 0.0;
    for (uint32_t i = 0; i < NQUAD; i++) {
        //    a = (d1*d1)*(f2-f3) + (d2*d2)*(f3-f1) + (d3*d3)*(f1-f2);
        //    b = d1*(f2-f3) + d2*(f3-f1) + d3*(f1-f2);
        //    dstar = 0.5 * (a/b);
        a = d1;
        b = d2;
        c = d3;
        top = (b-a)*(b-a)*(f2-f3) - (b-c)*(b-c)*(f2-f1); 
        bottom = (b-a)*(f2-f3) - (b-c)*(f2-f1);
        dstar = b - 0.5 * (top / bottom);

        //Make sure dstar is outside a certain tolerance of d2 (the center of the parabola)
        if (dstar < d2 + tol && dstar >= d2) dstar = d2 + tol;
        else if (dstar > d2 - tol && dstar <= d2) dstar = d2 - tol;

        if (isnan(dstar) || isinf(dstar)) {
            cout << "\t\tterminating loop 3 because dstar is NAN or INF" << endl;
            break;
        }

        // fs = evaluate(point + (dstar * step * direction));
        fs = evaluate_step(point, dstar * step, direction, current_point);
        evaluations_done++;

        cout << "\t\tloop 3, evaluations: " << evaluations_done << ", step: " << (dstar * step) << ", fitness: " << fs << ", dstar: " << dstar << endl;

        if (isnan(fs)) { status = "fs was NAN in loop 3"; throw status; }

        if (dstar > d2 ) {
            if (fs < f2) {
                d3 = dstar;
                f3 = fs;
            } else {
                d1 = d2;
                f1 = f2;
                d2 = dstar;
                f2 = fs;
            }
        } else {
            if (fs < f2) {
                d1 = dstar;
                f1 = fs;
            } else {
                d3 = d2;
                f3 = f2;
                d2 = dstar;
                f2 = fs;
            }
        }

        cout << "\t\tloop 3, evaluations: " << evaluations_done << ", step: " << (dstar * step) << ", fitness: " << f2 << "(f2 instead of fs -- should be minimum)" << endl;
        cout << "\t\t\td1:    " << d1    << ", f1: " << f1 << endl;
        cout << "\t\t\td2:    " << d2    << ", f2: " << f2 << endl;
        cout << "\t\t\td3:    " << d3    << ", f3: " << f3 << endl;
        cout << "\t\t\tdstar: " << dstar << ", fs: " << fs << endl;

        if (fabs(f1 - f2) < tol && fabs(f2 - f3) < tol && fabs(f1 - f3) < tol) {
//            ostringstream err_msg;
//            err_msg << "f1 - f2, f2 - f3, f1 - f3 all less than tolerance (" << tol << ") search finished.";
//            status = err_msg.str();
//            throw status;
            cout << "\t\tterminating loop 3 because f1 - f2, f2 - f3 and f1 - f3 all less than tolerance(" << tol << ")." << endl;
            break;
        }
    }

    new_point.resize(point.size(), 0.0);
    for (uint32_t i = 0; i < point.size(); i++) {
        new_point[i] = point[i] + (d2 * direction[i]);
    }

    new_fitness = f2;
    status = "success";
}

/*
int LineSearch::randomized_line_search(int number_parameters, double *point, double *step, int ls_evaluations, int ls_iterations, double **new_point, double *current_fitness) {
    int i, j, k;
    double *x, *y;
    double a, b, c;
    double a_error, b_error, c_error;
    double center, error;
    double *current;
    double min_step = -1;
    double max_step = 2;
    x = (double*)malloc(sizeof(double) * ls_evaluations);
    y = (double*)malloc(sizeof(double) * ls_evaluations);
    current = (double*)malloc(sizeof(double) * number_parameters);

    center = 0;
    for (k = 0; k < number_parameters; k++) current[k] = point[k];
    for (i = 0; i < ls_iterations; i++) {
        for (j = 0; j < ls_evaluations; j++) {
            x[j] = (drand48() * (max_step - min_step)) + min_step;
            for (k = 0; k < number_parameters; k++) current[k] = point[k] - (x[j] * step[k]);
            y[j] = evaluate(current);
        }

        parabolic_regression(ls_evaluations, x, y, &a, &b, &c, &a_error, &b_error, &c_error);
        printf("a: %.20lf, b: %.20lf, c: %.20lf, a_err: %.20lf, b_err: %.20lf, c_err: %.20lf\n", a, b, c, a_error, b_error, c_error);

        center = parabolic_center(a, b, c);
        error = parabolic_center(a_error, b_error, c_error);
        min_step = center - error;
        max_step = center + error;
        for (k = 0; k < number_parameters; k++) current[k] = point[k] - (center * step[k]);
        (*current_fitness) = evaluate(current);
        printf("line search iteration [%d], fitness: %.20lf, min/center/max: [%.20lf/%.20lf/%.20lf]\n\n", i, (*current_fitness), min_step, center, max_step);
    }
    (*new_point) = (double*)malloc(sizeof(double) * number_parameters);
    for (k = 0; k < number_parameters; k++) (*new_point)[k] = current[k];
    free(current);
    free(x);
    free(y);
}
*/
