/*
 * Copyright 2008, 2009 Travis Desell, Dave Przybylo, Nathan Cole,
 * Boleslaw Szymanski, Heidi Newberg, Carlos Varela, Malik Magdon-Ismail
 * and Rensselaer Polytechnic Institute.
 *
 * This file is part of Milkway@Home.
 *
 * Milkyway@Home is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Milkyway@Home is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Milkyway@Home.  If not, see <http://www.gnu.org/licenses/>.
 * */

#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "line_search.h"
#include "regression.h"
#include "../evaluation/evaluator.h"
#include "../util/io_util.h"

const char *LS_STR[] = {"success", "nan fitness in loop 1", "nan fitness in loop 2", "nan fitness in loop 3", "step < tol in loop 1", "eval max reached in loop 1", "eval max reached in loop 2"};

#define LOOP1_MAX    300
#define LOOP2_MAX    300
#define NQUAD        4
/********
    *    Stopping Conditions
 ********/
double        tol = 1e-6;

double evaluate_step(double* point, double step, double* direction, int number_parameters, double* current_point) {
    int i;
    for (i = 0; i < number_parameters; i++) {
        current_point[i] = point[i] + (step * direction[i]);
    }
    return evaluate(current_point);
}

int line_search(double* point, double initial_fitness, double* direction, int number_parameters, double* minimum, double* fitness, int *evaluations_done) {
    int i, jump, eval_count;
    double f1, f2, f3, fs;
    double d1, d2, d3, dstar;
    double step, a, b, c, top, bottom;
    double* current_point;

    current_point = (double*)malloc(sizeof(double) * number_parameters);

    printf("\tline search starting at fitness: %.15lf\n", initial_fitness);
    fwrite_double_array(stdout, "\tinitial point: ", number_parameters, point);
    /********
        *    Find f1 < f2
     ********/
    step = 1.0;
    f1 = initial_fitness;
    // f2 = evaluate( point + (direction * step) );
    f2 = evaluate_step(point, step, direction, number_parameters, current_point);
    (*evaluations_done) = 1;
    printf("\t\tloop 1, evaluations: %d, step: %.15lf, fitness: %.15lf\n", (*evaluations_done), step, f2);

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
    jump = 2.0;
    // f3 = evaluate( point + (d3 * step * direction) );
    f3 = evaluate_step(point, d3 * step, direction, number_parameters, current_point);
    (*evaluations_done)++;
    printf("\t\tloop 2, evaluations: %d, step: %.15lf, fitness: %.15lf\n", (*evaluations_done), d3 * step, f3);
    eval_count = 0;
    while (f3 >= f2 && !isnan(f1 - f2 - f3) && eval_count < LOOP2_MAX) {
        d1 = d2;
        f1 = f2;
        d2 = d3;
        f2 = f3;
        d3 = jump * d3;

        // f3 = evaluate( point + (d3 * step * direction) );
        f3 = evaluate_step(point, d3 * step, direction, number_parameters, current_point);
        (*evaluations_done)++;
        eval_count++;
        printf("\t\tloop 2, evaluations: %d, step: %.15lf, fitness: %.15lf\n", (*evaluations_done), d3 * step, f3);
    }
    if (isnan(f1 - f2 - f3)) return LS_LOOP2_NAN;
    if (eval_count == LOOP2_MAX) return LS_LOOP2_MAX;

    /********
        *    Find minimum
     ********/
    printf("\t\td1    %.15lf, f1: %.15lf\n", d1, f1);
    printf("\t\td2    %.15lf, f2: %.15lf\n", d2, f2);
    printf("\t\td3    %.15lf, f3: %.15lf\n", d3, f3);

    if (d1 < d2 && d2 < d3) {
        //Do nothing, this is the order we want things in.
    } else if (d1 > d2 && d2 > d3) {
        printf("swapping d1, d3 (and f1, f3), so d1, d2, d3 are in the correct order\n");
        //Swap d1, d3 (and f1, f3) so they are in order
        double temp;
        temp = d3;
        d3 = d1;
        d1 = temp;

        temp = f3;
        f3 = f1;
        f1 = temp;
    } else {
        fprintf(stderr, "ERROR: before 3rd phase of line search, d1, d2 and d3 were not in order");
        exit(0);
    }

    fs = 0.0;
    for (i = 0; i < NQUAD; i++) {
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

        // fs = evaluate(point + (dstar * step * direction));
        fs = evaluate_step(point, dstar * step, direction, number_parameters, current_point);
        (*evaluations_done)++;

        printf("\t\tloop 3, evaluations: %d, step: %.15lf, fitness: %.15lf, dstar: %.15lf\n", (*evaluations_done), (dstar * step), fs, dstar);
        if (isnan(fs)) return LS_LOOP3_NAN;

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
        //Now prints out "f2" instead of "fs"; f2 is updated midpoint, fs was test midpoint. -newbym2 
        printf("\t\tloop 3, evaluations: %d, step: %.15lf, fitness: %.15lf (f2 instead of fs)\n", (*evaluations_done), (dstar * step), f2);
        printf("\t\td1    %.15lf, f1: %.15lf\n", d1, f1);
        printf("\t\td2    %.15lf, f2: %.15lf\n", d2, f2);
        printf("\t\td3    %.15lf, f3: %.15lf\n", d3, f3);
        printf("\t\tdstar %.15lf, fs: %.15lf\n", dstar, fs);
    }

//    printf("\t\td1    %.15lf, f1: %.15lf\n", d1, f1);
//    printf("\t\td2    %.15lf, f2: %.15lf\n", d2, f2);
//    printf("\t\td3    %.15lf, f3: %.15lf\n", d3, f3);
//    printf("\t\tdstar %.15lf, fs: %.15lf\n", dstar, fs);

//  this is wrong, current point might not be the minimum.
//    memcpy(minimum, current_point, sizeof(double) * number_parameters);
    for (i = 0; i < number_parameters; i++) {
        minimum[i] = point[i] + (d2 * direction[i]);
    }

    (*fitness) = f2;

    free(current_point);
    return LS_SUCCESS;
}


void randomized_line_search(int number_parameters, double *point, double *step, int ls_evaluations, int ls_iterations, double **new_point, double *current_fitness) {
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
