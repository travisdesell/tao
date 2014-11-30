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

#ifndef TAO_PARTICLE_SWARM_H
#define TAO_PARTICLE_SWARM_H

#include <string>
#include <vector>

#include "asynchronous_algorithms/evolutionary_algorithm.hxx"

class ParticleSwarm : public EvolutionaryAlgorithm {
    protected:
        double inertia;
        double global_best_weight;
        double local_best_weight;
        double initial_velocity_scale;

        uint32_t initialized_individuals;

        std::vector< std::vector<double> > particles;
        std::vector< std::vector<double> > velocities;

        std::vector< std::vector<double> > local_bests;
        std::vector<double> local_best_fitnesses;

        double global_best_fitness;
        std::vector<double> global_best;

        ParticleSwarm();

        void initialize();
        void parse_arguments(const std::vector<std::string> &arguments);


    public:
        void (*print_statistics)(const std::vector<double> &);

        std::vector< std::vector<double> > get_population() { return local_bests; }
        std::vector< double > get_population_fitness() { return local_best_fitnesses; }

        std::vector< double> get_global_best() { return global_best; }
        double get_global_best_fitness() { return global_best_fitness; }

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

        ParticleSwarm( const std::vector<double> &min_bound,            /* min bound is copied into the search */
                       const std::vector<double> &max_bound,            /* max bound is copied into the search */
                       const uint32_t population_size,
                       const double inertia,                            /* intertia */
                       const double global_best_weight,                 /* global best weight */
                       const double local_best_weight,                  /* local best weight */
                       const double initial_velocity_scale,             /* A scale for the initial velocities of particles so it doesn't immediately go to the bounds */
                       const uint32_t maximum_created,                  /* default value is 0 which means no termination */
                       const uint32_t maximum_reported                  /* default value is 0 which means no termination */
                     ) throw (std::string);


        virtual ~ParticleSwarm();

        /**
         *  The following methods are used for asynchronous optimization
         */
        virtual void new_individual(uint32_t &id, std::vector<double> &parameters) throw (std::string);
        virtual void new_individual(uint32_t &id, std::vector<double> &parameters, uint32_t &seed) throw (std::string);
        virtual bool insert_individual(uint32_t id, const std::vector<double> &parameters, double fitness, uint32_t seed = 0) throw (std::string); /* Returns true if the individual is inserted. */
        virtual bool would_insert(uint32_t id, double fitness);

        /**
         *  The following method is for synchronous optimization 
         */
        void iterate(double (*objective_function)(const std::vector<double> &)) throw (std::string);
        void iterate(double (*objective_function)(const std::vector<double> &, const uint32_t)) throw (std::string);      //this objective function requires a seed

        void set_print_statistics(void (*_print_statistics)(const std::vector<double> &));

        virtual void get_individuals(std::vector<Individual> &individuals);
};

#endif
