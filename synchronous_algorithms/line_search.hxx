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

#ifndef TAO_LINE_SEARCH_H
#define TAO_LINE_SEARCH_H

#include <vector>
#include <string>
#include <iostream>

#include "stdint.h"

using std::string;
using std::vector;
using std::ostream;

class LineSearchException {
    private:
        uint32_t type;
        string message;

    public:
        const static uint32_t DIRECTION_BELOW_THRESHOLD = 1;

        const static uint32_t LOOP_2_MAX_REACHED = 2;

        const static uint32_t LOOP_2_F1_NAN = 3;
        const static uint32_t LOOP_2_F2_NAN = 4;
        const static uint32_t LOOP_2_F3_NAN = 5;
        const static uint32_t LOOP_3_FS_NAN = 6;

        const static uint32_t LOOP_2_F1_INF = 7;
        const static uint32_t LOOP_2_F2_INF = 8;
        const static uint32_t LOOP_2_F3_INF = 9;
        const static uint32_t LOOP_3_FS_INF = 10;

        const static uint32_t LOOP_2_OUT_OF_BOUNDS = 11;
        const static uint32_t LOOP_3_OUT_OF_BOUNDS = 12;

        LineSearchException(uint32_t type, string message);
        ~LineSearchException();

        uint32_t get_type();
        string get_message();

        void print_to(ostream &stream);
        friend std::ostream& operator<< (std::ostream& stream, LineSearchException &lse);
};


class LineSearch {
    private:
        /**
         *  Stopping Conditions
         */
        double tol;
        uint32_t LOOP1_MAX;
        uint32_t LOOP2_MAX;
        uint32_t NQUAD;

        bool threshold_specified;
        vector<double> min_threshold;

        double (*objective_function)(const vector<double> &);

        bool using_bounds;
        vector<double> min_bound;
        vector<double> max_bound;

    public:
        void parse_arguments(const vector<string> &arguments);

        LineSearch(double (*objective_function)(const vector<double> &));
        LineSearch(double (*objective_function)(const vector<double> &), vector<string> arguments);
        LineSearch(double (*objective_function)(const vector<double> &), const vector<double> &min_bound, const vector<double> &max_bound, vector<string> arguments);
        LineSearch(double (*objective_function)(const vector<double> &), const double tol, const uint32_t LOOP1_MAX, const uint32_t LOOP2_MAX, const uint32_t NQUAD);
        LineSearch(double (*objective_function)(const vector<double> &), const vector<double> &min_bound, const vector<double> &max_bound, const double tol, const uint32_t LOOP1_MAX, const uint32_t LOOP2_MAX, const uint32_t NQUAD);

        ~LineSearch();

        double evaluate_step(const vector<double> &point, const double step, const vector<double> &direction, vector<double> &current_point);

        void line_search(const vector<double> &point, double initial_fitness, const vector<double> &direction, vector <double> &new_point, double &new_fitness) throw (LineSearchException*);

//        int randomized_line_search(int number_parameters, double *point, double *step, int ls_evaluations, int ls_iterations, double **new_point, double *fitness);
};



#endif
