#ifndef TAO_DIFFERENTIAL_EVOLUTION_H
#define TAO_DIFFERENTIAL_EVOLUTION_H

#include <string>
#include <vector>

#include "evolutionary_algorithm.hxx"


class DifferentialEvolution : public EvolutionaryAlgorithm {
    protected:
        uint32_t number_pairs;
        uint16_t parent_selection;
        uint16_t recombination_selection;
        bool directional;

        double parent_scaling_factor;
        double differential_scaling_factor;
        double crossover_rate;

        std::vector<double> fitnesses;
        std::vector< std::vector<double> > population;

        uint32_t current_individual;
        uint32_t initialized_individuals;

        double global_best_fitness;
        uint32_t global_best_id;

        DifferentialEvolution();

        void initialize();
        void parse_arguments(const std::vector<std::string> &arguments);

    public:
        //The following are different types parent selection
        const static uint16_t PARENT_BEST = 0;
        const static uint16_t PARENT_RANDOM = 1;
        const static uint16_t PARENT_CURRENT_TO_BEST = 2;
        const static uint16_t PARENT_CURRENT_TO_RANDOM = 3;

        //The following are different types of recombination
        const static uint16_t RECOMBINATION_BINARY = 0;
        const static uint16_t RECOMBINATION_EXPONENTIAL = 1;
        const static uint16_t RECOMBINATION_SUM = 2;
        const static uint16_t RECOMBINATION_NONE = 3;

        DifferentialEvolution( const std::vector<std::string> &arguments) throw (std::string);

        DifferentialEvolution( const std::vector<double> &min_bound,                                    /* min bound is copied into the search */
                               const std::vector<double> &max_bound,                                    /* max bound is copied into the search */
                               const std::vector<std::string> &arguments) throw (std::string);          /* initialize the DE from command line arguments */

        DifferentialEvolution( const std::vector<double> &min_bound,                                    /* min bound is copied into the search */
                               const std::vector<double> &max_bound,                                    /* max bound is copied into the search */
                               const uint32_t population_size,
                               const uint16_t parent_selection,                                         /* How to select the parent */
                               const uint16_t number_pairs,                                             /* How many individuals to used to calculate differntials */
                               const uint16_t recombination_selection,                                  /* How to perform recombination */
                               const double parent_scaling_factor,                                      /* weight for the parent calculation*/
                               const double differential_scaling_factor,                                /* weight for the differential calculation */
                               const double crossover_rate,                                             /* crossover rate for recombination */
                               const bool directional,                                                  /* used for directional calculation of differential (this options is not really a recombination) */
                               const uint32_t maximum_iterations                                        /* default value is 0 which means no termination */
                             ) throw (std::string);

        DifferentialEvolution( const std::vector<double> &min_bound,                                    /* min bound is copied into the search */
                               const std::vector<double> &max_bound,                                    /* max bound is copied into the search */
                               const uint32_t population_size,
                               const uint16_t parent_selection,                                         /* How to select the parent */
                               const uint16_t number_pairs,                                             /* How many individuals to used to calculate differntials */
                               const uint16_t recombination_selection,                                  /* How to perform recombination */
                               const double parent_scaling_factor,                                      /* weight for the parent calculation*/
                               const double differential_scaling_factor,                                /* weight for the differential calculation */
                               const double crossover_rate,                                             /* crossover rate for recombination */
                               const bool directional,                                                  /* used for directional calculation of differential (this options is not really a recombination) */
                               const uint32_t maximum_created,                                          /* default value is 0 which means no termination */
                               const uint32_t maximum_reported                                          /* default value is 0 which means no termination */
                             ) throw (std::string);


        ~DifferentialEvolution();

        void parse_command_line(const std::vector<std::string> &arguments);
        /**
         *  The following methods are used for asynchronous optimization and are purely virtual
         */
        virtual void new_individual(uint32_t &id, std::vector<double> &parameters) throw (std::string);
        virtual void new_individual(uint32_t &id, std::vector<double> &parameters, uint32_t &seed) throw (std::string);
        virtual bool insert_individual(uint32_t id, const std::vector<double> &parameters, double fitness, uint32_t seed = 0) throw (std::string);     /* Returns true if the individual is inserted. */
        virtual bool would_insert(uint32_t id, double fitness);

        /**
         *  The following method is for synchronous optimization and is purely virtual
         */
        void iterate(double (*objective_function)(const std::vector<double> &)) throw (std::string);
        void iterate(double (*objective_function)(const std::vector<double> &, const uint32_t)) throw (std::string);    //this objective function also requires a seed
};


#endif
