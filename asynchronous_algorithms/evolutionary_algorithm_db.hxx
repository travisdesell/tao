/*
 * Copyright 2012, 2009 Travis Desell and the University of North Dakota.
 *
 * This file is part of the Toolkit for Asynchronous Optimization (TAO).
 *
 * TAO is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TAO is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TAO.  If not, see <http://www.gnu.org/licenses/>.
 * */

#ifndef TAO_EVOLUTIONARY_ALGORITHM_DB_H
#define TAO_EVOLUTIONARY_ALGORITHM_DB_H

#include "mysql.h"

#include <string>
#include <vector>

#include "stdint.h"

class EvolutionaryAlgorithmDB {
    protected:
        uint32_t id;
        int32_t app_id;
        std::string name;

    public:
        uint32_t get_id()        { return id; }
        std::string get_name()  { return name; }

        virtual void new_individual(uint32_t &id, std::vector<double> &parameters) throw (std::string) = 0;
        virtual void new_individual(uint32_t &id, std::vector<double> &parameters, uint32_t &seed) throw (std::string) = 0;

        virtual void update_current_individual() throw (std::string) = 0;

        virtual ~EvolutionaryAlgorithmDB() {
        }
};

#endif
