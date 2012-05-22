#ifndef TAO_EVOLUTIONARY_ALGORITHM_DB_H
#define TAO_EVOLUTIONARY_ALGORITHM_DB_H

#include "mysql.h"

#include <string>
#include <vector>

#include "stdint.h"

class EvolutionaryAlgorithmDB {
    protected:
        int id;
        std::string name;

    public:
        int get_id()            { return id; }
        std::string get_name()  { return name; }

        virtual void new_individual(uint32_t &id, std::vector<double> &parameters) throw (std::string) = 0;
        virtual void new_individual(uint32_t &id, std::vector<double> &parameters, uint32_t &seed) throw (std::string) = 0;

        virtual void update_current_individual() throw (std::string) = 0;
};

#endif
