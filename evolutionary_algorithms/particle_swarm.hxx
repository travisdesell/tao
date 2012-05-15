#ifndef TAO_PARTICLE_SWARM_H
#define TAO_PARTICLE_SWARM_H

#include <string>
#include <vector>

#include "evolutionary_algorithm.hxx"

class ParticleSwarm : public EvolutionaryAlgorithm {
    protected:
        double inertia;
        double global_best_weight;
        double local_best_weight;
        double initial_velocity_scale;

        uint32_t current_particle;

        std::vector< std::vector<double> > particles;
        std::vector< std::vector<double> > velocities;

        std::vector< std::vector<double> > local_bests;
        std::vector<double> local_best_fitnesses;

        double global_best_fitness;
        std::vector<double> global_best;

    public:
        ParticleSwarm( const std::vector<double> &min_bound,            /* min bound is copied into the search */
                       const std::vector<double> &max_bound,            /* max bound is copied into the search */
                       const std::vector<std::string> &arguments
                     ) throw (std::string);


        ParticleSwarm( const std::vector<double> &min_bound,            /* min bound is copied into the search */
                       const std::vector<double> &max_bound,            /* max bound is copied into the search */
                       const uint32_t population_size,
                       const double inertia                 = 0.95,     /* intertia */
                       const double global_best_weight      = 2.0,      /* global best weight */
                       const double local_best_weight       = 2.0,      /* local best weight */
                       const double initial_velocity_scale  = 0.1,      /* A scale for the initial velocities of particles so it doesn't immediately go to the bounds */
                       const uint32_t maximum_iterations    = 0         /* default value is 0 which means no termination */
                     ) throw (std::string);

        ~ParticleSwarm();

        /**
         *  The following methods are used for asynchronous optimization and are purely virtual
         */
        void new_individual(uint32_t &id, std::vector<double> &parameters) throw (std::string);
        void insert_individual(uint32_t id, const std::vector<double> &parameters, double fitness) throw (std::string);

        /**
         *  The following method is for synchronous optimization and is purely virtual
         */
        void iterate(double (*objective_function)(const std::vector<double> &)) throw (std::string);
};

#endif
