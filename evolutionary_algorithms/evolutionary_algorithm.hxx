#ifndef TAO_EVOLUTIONARY_ALGORITHM_H
#define TAO_EVOLUTIONARY_ALGORITHM_H

#include <vector>
#include <string>
#include <sstream>

#include <stdint.h>

class EvolutionaryAlgorithm {
    protected:
        uint32_t maximum_iterations;
        uint32_t current_iteration;
        uint32_t evaluations_done;

        uint32_t number_parameters;
        std::vector<double> min_bound;
        std::vector<double> max_bound;

        uint32_t population_size;

    public:
        /**
         *  Create/delete an EvolutionaryAlgorithm
         */
        EvolutionaryAlgorithm( const std::vector<double> &min_bound,        /* min bound is copied into the search */
                               const std::vector<double> &max_bound,        /* max bound is copied into the search */
                               const std::vector<std::string> &arguments    /* initialize the DE from command line arguments */
                             ) throw (std::string);

        EvolutionaryAlgorithm( const std::vector<double> &min_bound,    /* min bound is copied into the search */
                               const std::vector<double> &max_bound,    /* max bound is copied into the search */
                               const uint32_t population_size,
                               const uint32_t maximum_iterations = 0    /* default value is 0 which means no termination */
                             ) throw (std::string);


        ~EvolutionaryAlgorithm();

        /**
         *  The following methods are used for asynchronous optimization and are purely virtual
         */
        virtual void new_individual(uint32_t &id, std::vector<double> &parameters) throw (std::string) = 0;
        virtual void insert_individual(uint32_t id, const std::vector<double> &parameters, double fitness) throw (std::string) = 0;

        /**
         *  The following method is for synchronous optimization and is purely virtual
         */
        virtual void iterate(double (*objective_function)(const std::vector<double> &)) throw (std::string) = 0;
};

#endif
