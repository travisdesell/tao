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

        ParticleSwarm();

    public:
        ParticleSwarm( const std::vector<std::string> &arguments) throw (std::string);

        ParticleSwarm( const std::vector<double> &min_bound,            /* min bound is copied into the search */
                       const std::vector<double> &max_bound,            /* max bound is copied into the search */
                       const std::vector<std::string> &arguments
                     ) throw (std::string);


        ParticleSwarm( const std::vector<double> &min_bound,            /* min bound is copied into the search */
                       const std::vector<double> &max_bound,            /* max bound is copied into the search */
                       const uint32_t population_size,
                       const double inertia,                            /* intertia */
                       const double global_best_weight,                 /* global best weight */
                       const double local_best_weight,                  /* local best weight */
                       const double initial_velocity_scale,             /* A scale for the initial velocities of particles so it doesn't immediately go to the bounds */
                       const uint32_t maximum_iterations                /* default value is 0 which means no termination */
                     ) throw (std::string);

        ~ParticleSwarm();

        /**
         *  The following methods are used for asynchronous optimization
         */
        virtual void new_individual(uint32_t &id, std::vector<double> &parameters) throw (std::string);
        virtual void insert_individual(uint32_t id, const std::vector<double> &parameters, double fitness) throw (std::string);

        /**
         *  The following method is for synchronous optimization 
         */
        void iterate(double (*objective_function)(const std::vector<double> &)) throw (std::string);
};

#endif
