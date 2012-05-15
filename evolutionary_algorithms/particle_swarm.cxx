#include <string>
#include <vector>
#include <limits>
#include <iostream>
#include <iomanip>

#include "particle_swarm.hxx"
#include "recombination.hxx"

//From undvc_common
#include "vector_io.hxx"
#include "arguments.hxx"

using namespace std;

ParticleSwarm::ParticleSwarm(const vector<double> &min_bound, const vector<double> &max_bound, const vector<string> &arguments) throw (string) : EvolutionaryAlgorithm(min_bound, max_bound, arguments) {
    if (!get_argument(arguments, "--inertia", false, inertia)) {
        cerr << "Argument '--inertia <F>' not found, using default of 0.95." << endl;
        inertia = 0.95;
    }

    if (!get_argument(arguments, "--global_best_weight", false, global_best_weight)) {
        cerr << "Argument '--global_best_weight <F>' not found, using default of 2.0." << endl;
        global_best_weight = 2.0;
    }

    if (!get_argument(arguments, "--local_best_weight", false, local_best_weight)) {
        cerr << "Argument '--local_best_weight <F>' not found, using default of 2.0." << endl;
        local_best_weight = 2.0;
    }

    if (!get_argument(arguments, "--initial_velocity_scale", false, initial_velocity_scale)) {
        cerr << "Argument '--initial_velocity_scale <F>' not found, using default of 0.10." << endl;
        initial_velocity_scale = 0.10;
    }

    particles = vector< vector<double> >(population_size, vector<double>(number_parameters, 0.0));
    velocities = vector< vector<double> >(population_size, vector<double>(number_parameters, 0.0));

    local_bests = vector< vector<double> >(population_size, vector<double>(number_parameters, 0.0));
    local_best_fitnesses = vector<double>(population_size, -numeric_limits<double>::max());

    global_best_fitness = -numeric_limits<double>::max();
    global_best = vector<double>(number_parameters, 0);

    current_particle = 0;
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

    particles = vector< vector<double> >(population_size, vector<double>(number_parameters, 0.0));
    velocities = vector< vector<double> >(population_size, vector<double>(number_parameters, 0.0));

    local_bests = vector< vector<double> >(population_size, vector<double>(number_parameters, 0.0));
    local_best_fitnesses = vector<double>(population_size, -numeric_limits<double>::max());

    global_best_fitness = -numeric_limits<double>::max();
    global_best = vector<double>(number_parameters, 0);

    current_particle = 0;
}

ParticleSwarm::~ParticleSwarm() {
}

void
ParticleSwarm::new_individual(uint32_t &id, vector<double> &parameters) throw (string) {
    double r1 = drand48();  //TODO: use a better random number generator
    double r2 = drand48();  //TODO: use a better random number generator
//    cout << "r1: " << r1 << endl;
//    cout << "r2: " << r2 << endl;

    id = current_particle;
    current_particle++;
    if (current_particle >= population_size) current_particle = 0;

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

//        cout << "velocity   [" << j << "]: " << velocities[id][j] << endl;
//        cout << "particle   [" << j << "]: " << particles[id][j] << endl << endl;
    }
    parameters.assign(particles[id].begin(), particles[id].end());
}

void
ParticleSwarm::insert_individual(uint32_t id, const vector<double> &parameters, double fitness) throw (string) {
//    cout <<  current_iteration << ":" << i << " - NEW  : " << fitness << " [ " << vector_to_string(particles[i]) << " ]" << endl;

    /**
     *  TODO: do we want to also revert the particles current position to parameters, and it's velocity
     *  to what parameters velocity is?
     */
    if (fitness > local_best_fitnesses[id]) {
        local_best_fitnesses[id] = fitness;
//        for (uint32_t i = 0; i < velocities.size(); i++) velocities[id] = parameters[i] - local_bests[i];   //Rewind the velocity
        local_bests[id].assign(parameters.begin(), parameters.end());

//        cout.precision(15);
//        cout <<  current_iteration << ":" << id << " - LOCAL: " << fitness << " " << vector_to_string(parameters) << endl;
    }

    if (fitness > global_best_fitness) {
        global_best_fitness = fitness;
        global_best.assign(parameters.begin(), parameters.end());

        cout.precision(15);
        cout <<  current_iteration << ":" << setw(4) << id << " - GLOBAL: " << setw(-20) << fitness << " " << setw(-60) << vector_to_string(parameters) << ", velocity: " << setw(-60) << vector_to_string(velocities[id]) << endl;
    }

    evaluations_done++;
}

void
ParticleSwarm::iterate(double (*objective_function)(const vector<double> &)) throw (string) {
    srand48(time(NULL));    //TODO: probably use a different random number generator, maybe unique per EA

    //TODO: remove this and include it in new_individual, so asynchronous searches will properly initialize.

    //Initialize random population and velocities
    for (uint32_t i = 0; i < population_size; i++) {
        Recombination::random_parameters(min_bound, max_bound, particles[i]);
        Recombination::random_parameters(min_bound, max_bound, velocities[i]);

        local_bests[i].assign(particles[i].begin(), particles[i].end()); //local bests are initialized to copies of the particles

        //Set each velocity to the randomly generated position minus where the particle is at now (ie., each velocity
        //the difference between where the particle is now and some other random position in the search area)
        for (uint32_t j = 0; j < number_parameters; j++) {
            velocities[i][j] = initial_velocity_scale * (particles[i][j] - velocities[i][j]);
        }

        local_best_fitnesses[i] = objective_function(particles[i]);
        cout.precision(15);
//        cout <<  current_iteration << " - LOCAL: " << local_best_fitnesses[i] << " " << vector_to_string(particles[i]) << endl;

        if (local_best_fitnesses[i] > global_best_fitness) {
            global_best_fitness = local_best_fitnesses[i];
            global_best.assign(local_bests[i].begin(), local_bests[i].end());

            cout.precision(15);
//            cout <<  current_iteration << " - GLOBAL: " << global_best_fitness << " " << vector_to_string(particles[i]) << ", velocity: " << vector_to_string(velocities[i]) << endl;
        }
    }

    cout << "Initialized partilce swarm." << endl;
    cout << "   maximum_iterations: " << maximum_iterations << endl;
    cout << "   current_iteration:  " << current_iteration << endl;
    cout << "   inertia:            " << inertia << endl;
    cout << "   global_best_weight: " << global_best_weight << endl;
    cout << "   local_best_weight:  " << local_best_weight << endl;

    uint32_t id;
    vector<double> parameters;

    while (maximum_iterations == 0 || current_iteration < maximum_iterations) {
        for (uint32_t i = 0; i < population_size; i++) {
            new_individual(id, parameters);

            double fitness = objective_function(parameters);
            insert_individual(id, parameters, fitness);
        }

        current_iteration++;
    }
}

