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

#include <string>
#include <vector>
#include <limits>
#include <iostream>
#include <iomanip>

#include "asynchronous_algorithms/particle_swarm.hxx"
#include "asynchronous_algorithms/individual.hxx"

#include "util/arguments.hxx"
#include "util/recombination.hxx"
#include "util/statistics.hxx"
#include "util/vector_io.hxx"


using namespace std;


ParticleSwarm::ParticleSwarm() {
}

void
ParticleSwarm::initialize() {
    particles = vector< vector<double> >(population_size, vector<double>(number_parameters, 0.0));
    velocities = vector< vector<double> >(population_size, vector<double>(number_parameters, 0.0));

    local_bests = vector< vector<double> >(population_size, vector<double>(number_parameters, 0.0));
    local_best_fitnesses = vector<double>(population_size, -numeric_limits<double>::max());

    global_best_fitness = -numeric_limits<double>::max();
    global_best = vector<double>(number_parameters, 0);

    initialized_individuals = 0;
    current_individual = 0;

    start_time = time(NULL);
    print_statistics = NULL;
}

void
ParticleSwarm::set_print_statistics(void (*_print_statistics)(const std::vector<double> &)) {
    print_statistics = _print_statistics;
}

void
ParticleSwarm::parse_arguments(const vector<string> &arguments) {
    if (!get_argument(arguments, "--inertia", false, inertia)) {
        if (!quiet) cerr << "Argument '--inertia <F>' not found, using default of 0.75." << endl;
        inertia = 0.75;
    }

    if (!get_argument(arguments, "--global_best_weight", false, global_best_weight)) {
        if (!quiet) cerr << "Argument '--global_best_weight <F>' not found, using default of 1.5." << endl;
        global_best_weight = 1.5;
    }

    if (!get_argument(arguments, "--local_best_weight", false, local_best_weight)) {
        if (!quiet) cerr << "Argument '--local_best_weight <F>' not found, using default of 1.5." << endl;
        local_best_weight = 1.5;
    }

    if (!get_argument(arguments, "--initial_velocity_scale", false, initial_velocity_scale)) {
        if (!quiet) cerr << "Argument '--initial_velocity_scale <F>' not found, using default of 0.25." << endl;
        initial_velocity_scale = 0.25;
    }
}

ParticleSwarm::ParticleSwarm(const vector<string> &arguments) throw (string) : EvolutionaryAlgorithm(arguments) {
    parse_arguments(arguments);
    initialize();
}


ParticleSwarm::ParticleSwarm(const vector<double> &min_bound, const vector<double> &max_bound, const vector<string> &arguments) throw (string) : EvolutionaryAlgorithm(min_bound, max_bound, arguments) {
    parse_arguments(arguments);
    initialize();
}

ParticleSwarm::ParticleSwarm( const vector<double> &min_bound,                  /* min bound is copied into the search */
                              const vector<double> &max_bound,                  /* max bound is copied into the search */
                              const uint32_t population_size,
                              const double inertia,                             /* intertia */
                              const double global_best_weight,                  /* global best weight */
                              const double local_best_weight,                   /* local best weight */
                              const double initial_velocity_scale,              /* A scale for the initial velocities of particles so it doesn't immediately go to the bounds */
                              const uint32_t maximum_iterations                 /* default value is 0 which means no termination */
                            ) throw (string) : EvolutionaryAlgorithm(min_bound, max_bound, population_size, maximum_iterations) {
    this->inertia = inertia;
    this->global_best_weight = global_best_weight;
    this->local_best_weight = local_best_weight;
    this->initial_velocity_scale = initial_velocity_scale;

    initialize();
}

ParticleSwarm::ParticleSwarm( const vector<double> &min_bound,                  /* min bound is copied into the search */
                              const vector<double> &max_bound,                  /* max bound is copied into the search */
                              const uint32_t population_size,
                              const double inertia,                             /* intertia */
                              const double global_best_weight,                  /* global best weight */
                              const double local_best_weight,                   /* local best weight */
                              const double initial_velocity_scale,              /* A scale for the initial velocities of particles so it doesn't immediately go to the bounds */
                              const uint32_t maximum_created,                   /* default value is 0 which means no termination */
                              const uint32_t maximum_reported                   /* default value is 0 which means no termination */
                            ) throw (string) : EvolutionaryAlgorithm(min_bound, max_bound, population_size, maximum_created, maximum_reported) {
    this->inertia = inertia;
    this->global_best_weight = global_best_weight;
    this->local_best_weight = local_best_weight;
    this->initial_velocity_scale = initial_velocity_scale;

    initialize();
}


ParticleSwarm::~ParticleSwarm() {
}


void
ParticleSwarm::new_individual(uint32_t &id, vector<double> &parameters, uint32_t &seed) throw (string) {
    ParticleSwarm::new_individual(id, parameters);

    seeds[id] = (random_0_1(random_number_generator) * numeric_limits<uint32_t>::max()) / 10.0;    //uint max is too large for some reason
    seed = seeds[id];
}

void
ParticleSwarm::new_individual(uint32_t &id, vector<double> &parameters) throw (string) {
    id = current_individual;
    current_individual++;
    if (current_individual >= population_size) {
        current_individual = 0;
        current_iteration++;
    }

    //We haven't initialied all the particles so generate a random one
    if (initialized_individuals < particles.size()) {
        Recombination::random_within(min_bound, max_bound, particles[id], random_number_generator, random_0_1);
        Recombination::random_within(min_bound, max_bound, velocities[id], random_number_generator, random_0_1);

        //Set each velocity to the randomly generated position minus where the particle is at now (ie., each velocity
        //the difference between where the particle is now and some other random position in the search area)
        for (uint32_t j = 0; j < number_parameters; j++) {
            velocities[id][j] = initial_velocity_scale * (particles[id][j] - velocities[id][j]);
        }

        parameters.assign(particles[id].begin(), particles[id].end());
        individuals_created++;
        return;
    }

    double r1 = random_0_1(random_number_generator);
    double r2 = random_0_1(random_number_generator);
//    cout << "r1: " << r1 << endl;
//    cout << "r2: " << r2 << endl;

    for (uint32_t j = 0; j < number_parameters; j++) {
//        cout << "particle(b)[" << j << "]: " << particles[id][j] << endl;
//        cout << "velocity(b)[" << j << "]: " << velocities[id][j] << endl;
//        cout << "global best[" << j << "]: " << global_best[j] << endl;
//        cout << "local  best[" << j << "]: " << local_bests[id][j] << endl;

        double modified_velocity = inertia * velocities[id][j];
        double global_pull = global_best_weight * r1 * (global_best[j] - particles[id][j]);
        double local_pull = local_best_weight * r2 * (local_bests[id][j] - particles[id][j]);

//        cout << "modvelocity[" << j << "]: " << modified_velocity << endl;
//        cout << "global_pull[" << j << "]: " << global_pull << endl;
//        cout << "local_pull [" << j << "]: " << local_pull << endl;

        velocities[id][j] = modified_velocity + global_pull + local_pull;

        if (wrap_radians && (fabs(max_bound[j] - (2 * M_PI)) < 0.00001) && (fabs(min_bound[j] - (-2 * M_PI)) < 0.00001) ) {
            double next_position = particles[id][j] + velocities[id][j];
//            cout << "\tbounding radian start: " << next_position << endl;

            while (next_position > max_bound[j]) next_position -= (2 * M_PI);
            while (next_position < min_bound[j]) next_position += (2 * M_PI);

//            cout << "\tbounding radian end:   " << next_position << endl;

            particles[id][j] = next_position;

        } else {
            //Enforce bounds
            if (particles[id][j] + velocities[id][j] > max_bound[j]) {
                velocities[id][j] = max_bound[j] - particles[id][j];
                particles[id][j] = max_bound[j];
            } else if (particles[id][j] + velocities[id][j] < min_bound[j]) {
                velocities[id][j] = particles[id][j] - min_bound[j];
                particles[id][j] = min_bound[j];
            } else {
                particles[id][j] += velocities[id][j];
            }
        }

//        cout << "velocity   [" << j << "]: " << velocities[id][j] << endl;
//        cout << "particle   [" << j << "]: " << particles[id][j] << endl << endl;
    }
    parameters.assign(particles[id].begin(), particles[id].end());

    individuals_created++;
}

bool
ParticleSwarm::insert_individual(uint32_t id, const vector<double> &parameters, double fitness, uint32_t seed) throw (string) {
    bool modified = false;
//    cout <<  current_iteration << ":" << i << " - NEW  : " << fitness << " [ " << vector_to_string(particles[i]) << " ]" << endl;

    /**
     *  TODO: do we want to also revert the particles current position to parameters, and it's velocity
     *  to what parameters velocity is?
     */

    if (local_best_fitnesses[id] < fitness) {
        if (local_best_fitnesses[id] == -numeric_limits<double>::max()) initialized_individuals++;

        local_best_fitnesses[id] = fitness;
//        for (uint32_t i = 0; i < velocities.size(); i++) velocities[id] = parameters[i] - local_bests[i];   //Rewind the velocity
        local_bests[id].assign(parameters.begin(), parameters.end());

//        cout.precision(10);
//        cout <<  current_iteration << ":" << id << " - LOCAL: " << fitness << " " << vector_to_string(parameters) << endl;

        /*
        if (log_file != NULL) {
            double best, average, median, worst;
            calculate_fitness_statistics(local_best_fitnesses, best, average, median, worst);
            (*log_file) << individuals_reported << " -- b: " << best << ", a: " << average << ", m: " << median << ", w: " << worst << endl;
        }
        */

        modified = true;
    }

    if (global_best_fitness < fitness) {
        global_best_fitness = fitness;
        global_best.assign(parameters.begin(), parameters.end());

        if (log_file == NULL) {
            cout.precision(10);
            if (!quiet) {
                cout << current_iteration << ":" << setw(4) << id << " - GLOBAL: " << setw(-20) << fitness << " " << setw(-60) << vector_to_string(parameters) << ", velocity: " << setw(-60) << vector_to_string(velocities[id]) << endl;
            }
        } else {
            double best, average, median, worst;
            calculate_fitness_statistics(local_best_fitnesses, best, average, median, worst);
            (*log_file) << individuals_reported << " -- b: " << best << ", a: " << average << ", m: " << median << ", w: " << worst << ", " << vector_to_string(parameters) << endl;
        }
    }

    individuals_reported++;
    return modified;
}

bool
ParticleSwarm::would_insert(uint32_t id, double fitness) {
    return local_best_fitnesses[id] < fitness;
}


void
ParticleSwarm::iterate(double (*objective_function)(const vector<double> &)) throw (string) {
    if (!quiet) {
        cout << "Initialized partilce swarm." << endl;
        cout << "   maximum_iterations: " << maximum_iterations << endl;
        cout << "   current_iteration:  " << current_iteration << endl;
        cout << "   inertia:            " << inertia << endl;
        cout << "   global_best_weight: " << global_best_weight << endl;
        cout << "   local_best_weight:  " << local_best_weight << endl;
    }

    uint32_t id;
    vector<double> parameters(number_parameters, 0);

    while (maximum_iterations == 0 || current_iteration < maximum_iterations) {
        for (uint32_t i = 0; i < population_size; i++) {
            new_individual(id, parameters);

            double fitness = objective_function(parameters);
            insert_individual(id, parameters, fitness);
        }

        current_iteration++;
    }
}

void
ParticleSwarm::iterate(double (*objective_function)(const vector<double> &, const uint32_t)) throw (string) {
    if (!quiet) {
        cout << "Initialized particle swarm." << endl;
        cout << "   maximum_iterations: " << maximum_iterations << endl;
        cout << "   current_iteration:  " << current_iteration << endl;
        cout << "   inertia:            " << inertia << endl;
        cout << "   global_best_weight: " << global_best_weight << endl;
        cout << "   local_best_weight:  " << local_best_weight << endl;
    }

    uint32_t id;
    uint32_t seed;
    vector<double> parameters(number_parameters, 0);

    while (maximum_iterations == 0 || current_iteration < maximum_iterations) {
        for (uint32_t i = 0; i < population_size; i++) {
            new_individual(id, parameters, seed);

            double fitness = objective_function(parameters, seed);
            insert_individual(id, parameters, fitness, seed);
        }

        //This is now updated in the 'new_individual' function.
//        current_iteration++;
    }
}

void
ParticleSwarm::get_individuals(vector<Individual> &individuals) {
    individuals.clear();
    for (uint32_t i = 0; i < population_size; i++) {
        individuals.push_back(Individual(i, local_best_fitnesses[i], local_bests[i], ""));
    }
}
