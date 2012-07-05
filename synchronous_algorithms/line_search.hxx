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

#include "stdint.h"

using std::string;
using std::vector;

class LineSearch {
    private:
        string status;

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

    public:
        LineSearch(double (*objective_function)(const vector<double> &));
        LineSearch(double (*objective_function)(const vector<double> &), vector<string> arguments);
        LineSearch(double (*objective_function)(const vector<double> &), const double tol, const uint32_t LOOP1_MAX, const uint32_t LOOP2_MAX, const uint32_t NQUAD);

        ~LineSearch();

        string get_status();

        double evaluate_step(const vector<double> &point, const double step, const vector<double> &direction, vector<double> &current_point);

        void line_search(const vector<double> &point, double initial_fitness, const vector<double> &direction, vector <double> &new_point, double &new_fitness) throw (string);

//        int randomized_line_search(int number_parameters, double *point, double *step, int ls_evaluations, int ls_iterations, double **new_point, double *fitness);
};



#endif
