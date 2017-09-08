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

//#include "util/regression.hxx"
#include "util/arguments.hxx"
#include "util/recombination.hxx"
#include "util/vector_io.hxx"

using std::string;
using std::ostringstream;
using std::vector;
using std::cout;
using std::endl;

LineSearchException::LineSearchException(uint32_t type, string message) {
    this->type = type;
    this->message = message;
}

LineSearchException::~LineSearchException() {
}

uint32_t LineSearchException::get_type() {
    return type;
}

string LineSearchException::get_message() {
    return message;
}

void LineSearchException::print_to(ostream& stream) {
    stream << "'" << message.c_str() << "'";
}

ostream& operator<< (ostream& stream, LineSearchException &lse) {
    lse.print_to(stream);
    return stream;
}

bool ls_quiet = false;

void LineSearch::parse_arguments(const vector<string> &arguments) {
    if (argument_exists(arguments, "--gd_quiet")) {
        ls_quiet = true;
    }

    if (!get_argument(arguments, "--loop1_max", false, LOOP1_MAX)) {
        if (!ls_quiet) cerr << "Argument '--loop1_max' not found, using default of 300 maximum iterations for loop 1 of the line search." << endl;
        LOOP1_MAX = 300;
    }

    if (!get_argument(arguments, "--loop2_max", false, LOOP2_MAX)) {
        if (!ls_quiet) cerr << "Argument '--loop2_max' not found, using default of 300 maximum iterations for loop 2 of the line search." << endl;
        LOOP2_MAX = 300;
    }

    if (!get_argument(arguments, "--nquad", false, NQUAD)) {
        if (!ls_quiet) cerr << "Argument '--nquad <i>' not found, using default of 4 iterations for loop 3 of the line search." << endl;

        NQUAD = 4;
    }

    if (!get_argument(arguments, "--tol", false, tol)) {
        if (!ls_quiet) cerr << "Argument '--tol <d>' not found, using default of 1e-6 for tolerance of dstar in loop 3 of the line search." << endl;
        tol = 1e-6;
    }

    threshold_specified = get_argument_vector(arguments, "--min_threshold", false, min_threshold);

    if (!threshold_specified) {
        if (!ls_quiet) cerr << "Argument '--min_threshold <d1, d2 .. dn>' not found. line search will not quit if the input direction is very small." << endl;
    }
}

LineSearch::LineSearch(double (*objective_function)(const vector<double> &), vector<string> arguments) {
    this->objective_function = objective_function;
    parse_arguments(arguments);

    using_bounds = true;
    if (!get_argument_vector(arguments, "--min_bound", false, min_bound) || !get_argument_vector(arguments, "--max_bound", false, max_bound)) {
        if (!ls_quiet) cerr << "Arguments '--min_bound <d1 .. dn>' and '--max_bound <d1 .. dn>' not found. line search will not be bounded." << endl;
        using_bounds = false;
    }

}

LineSearch::LineSearch(double (*objective_function)(const vector<double> &), const vector<double> &min_bound, const vector<double> &max_bound, vector<string> arguments) {
    this->objective_function = objective_function;
    parse_arguments(arguments);

    using_bounds = true;
    this->min_bound.assign(min_bound.begin(), min_bound.end());
    this->max_bound.assign(max_bound.begin(), max_bound.end());
}


LineSearch::LineSearch(double (*objective_function)(const vector<double> &), const double tol, const uint32_t LOOP1_MAX, const uint32_t LOOP2_MAX, const uint32_t NQUAD) {
    this->objective_function = objective_function;
    this->tol = tol;
    this->LOOP1_MAX = LOOP1_MAX;
    this->LOOP2_MAX = LOOP2_MAX;
    this->NQUAD = NQUAD;

    using_bounds = false;
}

LineSearch::LineSearch(double (*objective_function)(const vector<double> &), const vector<double> &min_bound, const vector<double> &max_bound, const double tol, const uint32_t LOOP1_MAX, const uint32_t LOOP2_MAX, const uint32_t NQUAD) {
    this->objective_function = objective_function;
    this->tol = tol;
    this->LOOP1_MAX = LOOP1_MAX;
    this->LOOP2_MAX = LOOP2_MAX;
    this->NQUAD = NQUAD;

    using_bounds = true;
    this->min_bound.assign(min_bound.begin(), min_bound.end());
    this->max_bound.assign(max_bound.begin(), max_bound.end());
}


LineSearch::~LineSearch() {
}

double LineSearch::evaluate_step(const vector<double> &point, const double step, const vector<double> &direction, vector<double> &current_point) {
    for (uint32_t i = 0; i < point.size(); i++) {
        current_point[i] = point[i] + (step * direction[i]);
    }
    return objective_function(current_point);
}

void LineSearch::line_search(const vector<double> &point, double initial_fitness, const vector<double> &direction, vector <double> &new_point, double &new_fitness) throw (LineSearchException*) {
    if (threshold_specified && gradient_below_threshold(direction, min_threshold)) {
        if (!ls_quiet) cerr << "\tDirection dropped below threshold:" << endl;
        if (!ls_quiet) cerr << "\tdirection:  " << vector_to_string(direction) << endl;
        if (!ls_quiet) cerr << "\tthreshold: " << vector_to_string(min_threshold) << endl;

        throw new LineSearchException(LineSearchException::DIRECTION_BELOW_THRESHOLD, "direction dropped below threshold");
    }

    uint32_t evaluations_done;
    double d1, d2, d3, dstar;
    double a, b, c, top, bottom;

    vector<double> current_point(point);

    if (!ls_quiet) cout << "\tline search starting at fitness: " << initial_fitness << endl;
    if (!ls_quiet) cout << "\tinitial_point: " << vector_to_string(current_point) << endl;

    /********
        *    Find f1 < f2
     ********/
    double step = 1.0;
    double f1 = initial_fitness;
    // f2 = evaluate( point + (direction * step) );
    double f2 = evaluate_step(point, step, direction, current_point);
    evaluations_done = 1;

    if (!ls_quiet) cout << "\t\tloop 1, evaluations: " << evaluations_done << ", step: " << step << ", fitness: " << f2 << endl;

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

    if (!ls_quiet) cout << "\t\tloop 2, evaluations: " << evaluations_done << ", step: " << (d3 * step) << ", fitness: " << f3 << endl;

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

        if (!ls_quiet) cout << "\t\tloop 2, evaluations: " << evaluations_done << ", step: " << (d3 * step) << ", fitness: " << f3 << endl;

        if ( using_bounds && Recombination::out_of_bounds(min_bound, max_bound, current_point) ) {
            new_point.resize(point.size(), 0.0);
            for (uint32_t i = 0; i < point.size(); i++) {
                new_point[i] = point[i] + (d3 * direction[i]);
            }
            Recombination::bound_parameters(min_bound, max_bound, new_point);
            new_fitness = objective_function(new_point);

            throw new LineSearchException(LineSearchException::LOOP_2_OUT_OF_BOUNDS, "parameters out of bounds in loop 2");
        }
    }

    if (isnan(f1)) throw new LineSearchException(LineSearchException::LOOP_2_F1_NAN, "f1 was NAN in loop 2"); 
    if (isnan(f2)) throw new LineSearchException(LineSearchException::LOOP_2_F2_NAN, "f2 was NAN in loop 2"); 
    if (isnan(f3)) throw new LineSearchException(LineSearchException::LOOP_2_F3_NAN, "f3 was NAN in loop 2"); 
    if (isinf(f1)) throw new LineSearchException(LineSearchException::LOOP_2_F1_INF, "f1 was INF in loop 2"); 
    if (isinf(f2)) throw new LineSearchException(LineSearchException::LOOP_2_F2_INF, "f2 was INF in loop 2"); 
    if (isinf(f3)) throw new LineSearchException(LineSearchException::LOOP_2_F3_INF, "f3 was INF in loop 2"); 

    if (eval_count >= LOOP2_MAX) {
        ostringstream ex_msg;
        ex_msg << "loop 2 reached maximum evaluation count: " << eval_count;

        throw new LineSearchException(LineSearchException::LOOP_2_MAX_REACHED, ex_msg.str());
    }


    /********
        *    Find minimum
     ********/
    if (!ls_quiet) {
        cout << "\t\tNeed d1 < d2 < d3" << endl;
        cout << "\t\t\td1:    " << d1 << ", f1: " << f1 << endl;
        cout << "\t\t\td2:    " << d2 << ", f2: " << f2 << endl;
        cout << "\t\t\td3:    " << d3 << ", f3: " << f3 << endl;
    }

    if (d1 < d2 && d2 < d3) {
        //Do nothing, this is the order we want things in.
    } else if (d1 > d2 && d2 > d3) {
        if (!ls_quiet) cout << "\t\tswapping d1, d3 (and f1, f3), so d1, d2, d3 are in the correct order." << endl;
        //Swap d1, d3 (and f1, f3) so they are in order
        double temp;
        temp = d3;
        d3 = d1;
        d1 = temp;

        temp = f3;
        f3 = f1;
        f1 = temp;
    } else {
        if (!ls_quiet) cerr <<  "ERROR: before 3rd phase of line search, d1, d2 and d3 were not in order." << endl; //This should never happen
        exit(1);
    }

    if (!ls_quiet) {
        cout << "\t\tShould be d1 < d2 < d3:" << endl;
        cout << "\t\t\td1:    " << d1 << ", f1: " << f1 << endl;
        cout << "\t\t\td2:    " << d2 << ", f2: " << f2 << endl;
        cout << "\t\t\td3:    " << d3 << ", f3: " << f3 << endl;
    }

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
            if (!ls_quiet) cout << "\t\tterminating loop 3 because dstar is NAN or INF" << endl;
            break;
        }

        // fs = evaluate(point + (dstar * step * direction));
        fs = evaluate_step(point, dstar * step, direction, current_point);
        evaluations_done++;

        if (!ls_quiet) cout << "\t\tloop 3, evaluations: " << evaluations_done << ", step: " << (dstar * step) << ", fitness: " << fs << ", dstar: " << dstar << endl;

        if ( using_bounds && Recombination::out_of_bounds(min_bound, max_bound, current_point) ) {
            new_point.resize(point.size(), 0.0);
            for (uint32_t i = 0; i < point.size(); i++) {
                new_point[i] = point[i] + (dstar * direction[i]);
            }
            Recombination::bound_parameters(min_bound, max_bound, new_point);
            new_fitness = objective_function(new_point);

            throw new LineSearchException(LineSearchException::LOOP_3_OUT_OF_BOUNDS, "parameters out of bounds in loop 3");
        }

        if (isnan(fs)) throw new LineSearchException(LineSearchException::LOOP_3_FS_NAN, "fs was NAN in loop 3"); 
        if (isinf(fs)) throw new LineSearchException(LineSearchException::LOOP_3_FS_INF, "fs was INF in loop 3"); 

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

        if (!ls_quiet) {
            cout << "\t\tloop 3, evaluations: " << evaluations_done << ", step: " << (dstar * step) << ", fitness: " << f2 << "(f2 instead of fs -- should be minimum)" << endl;
            cout << "\t\t\td1:    " << d1    << ", f1: " << f1 << endl;
            cout << "\t\t\td2:    " << d2    << ", f2: " << f2 << endl;
            cout << "\t\t\td3:    " << d3    << ", f3: " << f3 << endl;
            cout << "\t\t\tdstar: " << dstar << ", fs: " << fs << endl;
        }

        if (fabs(f1 - f2) < tol && fabs(f2 - f3) < tol && fabs(f1 - f3) < tol) {
//            ostringstream err_msg;
//            err_msg << "f1 - f2, f2 - f3, f1 - f3 all less than tolerance (" << tol << ") search finished.";
            if (!ls_quiet) cout << "\t\tterminating loop 3 because f1 - f2, f2 - f3 and f1 - f3 all less than tolerance(" << tol << ")." << endl;
            break;
        }
    }

    new_point.resize(point.size(), 0.0);
    for (uint32_t i = 0; i < point.size(); i++) {
        new_point[i] = point[i] + (d2 * direction[i]);
    }
    new_fitness = f2;
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
