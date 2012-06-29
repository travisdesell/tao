#include <string>
#include <vector>
#include <limits>
#include <iostream>

#include "evolutionary_algorithm.hxx"
#include "differential_evolution.hxx"
#include "recombination.hxx"
#include "statistics.hxx"

//from undvc_common
#include "vector_io.hxx"
#include "arguments.hxx"

using namespace std;

/**
 *  Initialize a differential evolution search from command line parameters
 */

DifferentialEvolution::DifferentialEvolution() {
}

void
DifferentialEvolution::parse_arguments(const vector<string> &arguments) {
    string parent_selection_name, recombination_selection_name;

    if (!get_argument(arguments, "--parent_scaling_factor", false, parent_scaling_factor)) {
        cerr << "Argument '--parent_scaling_factor <F>' not found, using default of 1.0." << endl;
        parent_scaling_factor = 1.0;
    }

    if (!get_argument(arguments, "--differential_scaling_factor", false, differential_scaling_factor)) {
        cerr << "Argument '--differential_scaling_factor <F>' not found, using default of 1.0." << endl;
        differential_scaling_factor = 1.0;
    }

    if (!get_argument(arguments, "--crossover_rate", false, crossover_rate)) {
        cerr << "Argument '--crossover_rate <F>' not found, using default of 0.5." << endl;
        crossover_rate = 0.5;
    }

    if (!get_argument(arguments, "--number_pairs", false, number_pairs)) {
        cerr << "Argument '--number_pairs <I>' not found, using default of 1." << endl;
        number_pairs = 1;
    }

    if (get_argument(arguments, "--parent_selection", false, parent_selection_name)) {
        if (parent_selection_name.compare("best") == 0) {
            parent_selection = PARENT_BEST;
        } else if (parent_selection_name.compare("random") == 0) {
            parent_selection = PARENT_RANDOM;
        } else if (parent_selection_name.compare("current-to-best") == 0) {
            parent_selection = PARENT_CURRENT_TO_BEST;
        } else if (parent_selection_name.compare("current-to-random") == 0) {
            parent_selection = PARENT_CURRENT_TO_RANDOM;
        } else {
            cerr << "Improperly specified parent selection type: '" << parent_selection_name.c_str() << "'" << endl;
            cerr << "Possibilities are:" << endl;
            cerr << "   best" << endl;
            cerr << "   random" << endl;
            cerr << "   current-to-best" << endl;
            cerr << "   current-to-random" << endl;
            exit(1);
        }   
    } else {
        cerr << "Argument '--parent_selection <S>' not found, using default of 'best'." << endl;
        parent_selection = PARENT_BEST;
    }

    if (get_argument(arguments, "--recombination_selection", false, recombination_selection_name)) {
        if (recombination_selection_name.compare("binary") == 0) {
            recombination_selection = RECOMBINATION_BINARY;
        } else if (recombination_selection_name.compare("exponential") == 0) {
            recombination_selection = RECOMBINATION_EXPONENTIAL;
        } else if (recombination_selection_name.compare("sum") == 0) {
            recombination_selection = RECOMBINATION_SUM;
        } else if (recombination_selection_name.compare("none") == 0) {
            recombination_selection = RECOMBINATION_NONE;
        } else {
            cerr << "Improperly specified recombination type: '%s'" << parent_selection_name.c_str() << "'" << endl;
            cerr << "Possibilities are:" << endl;
            cerr << "   binary" << endl;
            cerr << "   exponential" << endl;
            cerr << "   sum" << endl;
            cerr << "   none" << endl;
            exit(1);
        }   
    } else {
        cerr << "Argument '--recombination_selection <S>' not found, using default of 'binary'." << endl;
        recombination_selection = RECOMBINATION_BINARY;
    }

    directional = argument_exists(arguments, "directional");
}

void
DifferentialEvolution::initialize() {
    this->current_individual = 0;
    this->initialized_individuals = 0;

    this->global_best_fitness = -numeric_limits<double>::max();
    this->global_best_id = 0;

    population = vector< vector<double> >(population_size, vector<double>(number_parameters, 0.0));
    fitnesses = vector<double>(population_size, -numeric_limits<double>::max());
}

DifferentialEvolution::DifferentialEvolution(const vector<string> &arguments) throw (string) : EvolutionaryAlgorithm(arguments) {
    parse_arguments(arguments);
    initialize();
}

DifferentialEvolution::DifferentialEvolution(const vector<double> &min_bound, const vector<double> &max_bound, const vector<string> &arguments) throw (string) : EvolutionaryAlgorithm(min_bound, max_bound, arguments) {
    parse_arguments(arguments);
    initialize();
}

DifferentialEvolution::DifferentialEvolution( const std::vector<double> &min_bound,         /* min bound is copied into the search */
                                              const std::vector<double> &max_bound,         /* max bound is copied into the search */
                                              const uint32_t population_size,
                                              const uint16_t parent_selection,              /* How to select the parent */
                                              const uint16_t number_pairs,                  /* How many individuals to used to calculate differentials */
                                              const uint16_t recombination_selection,       /* How to perform recombination */
                                              const double parent_scaling_factor,           /* weight for the parent calculation*/
                                              const double differential_scaling_factor,     /* weight for the differential calculation */
                                              const double crossover_rate,                  /* crossover rate for recombination */
                                              const bool directional,                       /* used for directional calculation of differential (this options is not really a recombination) */
                                              const uint32_t maximum_iterations             /* default value is 0 which means no termination */
                                            ) throw (std::string) : EvolutionaryAlgorithm(min_bound, max_bound, population_size, maximum_iterations) {

    if (parent_selection != PARENT_BEST && parent_selection != PARENT_RANDOM && parent_selection != PARENT_CURRENT_TO_BEST && parent_selection != PARENT_CURRENT_TO_RANDOM) {
        std::stringstream oss;
        oss << "ERROR [file: " << __FILE__ << ", line: " << __LINE__ << "]: unknown parent selection type (" << parent_selection << ")";
        throw oss.str();
    }

    if (recombination_selection != RECOMBINATION_BINARY && recombination_selection != RECOMBINATION_EXPONENTIAL && recombination_selection != RECOMBINATION_SUM && recombination_selection != RECOMBINATION_NONE) {
        std::stringstream oss;
        oss << "ERROR [file: " << __FILE__ << ", line: " << __LINE__ << "]: unknown recombination selection type (" << parent_selection << ")";
        throw oss.str();
    }

    this->parent_selection = parent_selection;
    this->number_pairs = number_pairs;
    this->recombination_selection = recombination_selection;
    this->parent_scaling_factor = parent_scaling_factor;
    this->differential_scaling_factor = differential_scaling_factor;
    this->crossover_rate = crossover_rate;
    this->directional = directional;

    maximum_created = 0;
    maximum_reported = 0;
    this->maximum_iterations = maximum_iterations;

    initialize();
}

DifferentialEvolution::DifferentialEvolution( const std::vector<double> &min_bound,         /* min bound is copied into the search */
                                              const std::vector<double> &max_bound,         /* max bound is copied into the search */
                                              const uint32_t population_size,
                                              const uint16_t parent_selection,              /* How to select the parent */
                                              const uint16_t number_pairs,                  /* How many individuals to used to calculate differentials */
                                              const uint16_t recombination_selection,       /* How to perform recombination */
                                              const double parent_scaling_factor,           /* weight for the parent calculation*/
                                              const double differential_scaling_factor,     /* weight for the differential calculation */
                                              const double crossover_rate,                  /* crossover rate for recombination */
                                              const bool directional,                       /* used for directional calculation of differential (this options is not really a recombination) */
                                              const uint32_t maximum_created,               /* default value is 0 which means no termination */
                                              const uint32_t maximum_reported               /* default value is 0 which means no termination */
                                            ) throw (std::string) : EvolutionaryAlgorithm(min_bound, max_bound, population_size, maximum_iterations) {

    if (parent_selection != PARENT_BEST && parent_selection != PARENT_RANDOM && parent_selection != PARENT_CURRENT_TO_BEST && parent_selection != PARENT_CURRENT_TO_RANDOM) {
        std::stringstream oss;
        oss << "ERROR [file: " << __FILE__ << ", line: " << __LINE__ << "]: unknown parent selection type (" << parent_selection << ")";
        throw oss.str();
    }

    if (recombination_selection != RECOMBINATION_BINARY && recombination_selection != RECOMBINATION_EXPONENTIAL && recombination_selection != RECOMBINATION_SUM && recombination_selection != RECOMBINATION_NONE) {
        std::stringstream oss;
        oss << "ERROR [file: " << __FILE__ << ", line: " << __LINE__ << "]: unknown recombination selection type (" << parent_selection << ")";
        throw oss.str();
    }

    this->parent_selection = parent_selection;
    this->number_pairs = number_pairs;
    this->recombination_selection = recombination_selection;
    this->parent_scaling_factor = parent_scaling_factor;
    this->differential_scaling_factor = differential_scaling_factor;
    this->crossover_rate = crossover_rate;
    this->directional = directional;

    this->maximum_created = maximum_created;
    this->maximum_reported = maximum_reported;
    maximum_iterations = 0;

    initialize();
}


DifferentialEvolution::~DifferentialEvolution() {
}

void
DifferentialEvolution::new_individual(uint32_t &id, std::vector<double> &parameters, uint32_t &seed) throw (string) {
    DifferentialEvolution::new_individual(id, parameters);

    seeds[id] = ((*random_number_generator)() * numeric_limits<uint32_t>::max()) / 10.0;    //uint max is too large for some reason
    seed = seeds[id];
}

void
DifferentialEvolution::new_individual(uint32_t &id, std::vector<double> &parameters) throw (string) {
    id = current_individual;
    current_individual++;
    if (current_individual >= population_size) current_individual = 0;

    if (initialized_individuals < population_size) { //The search has not been fully initalized so keep generating random individuals
        Recombination::random_parameters(min_bound, max_bound, parameters, random_number_generator);
        population[id].assign(parameters.begin(), parameters.end());
        individuals_created++;
        return;
    }

    /**
     *  Select the parent.
     */
    vector<double> parent(number_parameters, 0);

    switch (parent_selection) {
        case PARENT_BEST:
            parent.assign(population[global_best_id].begin(), population[global_best_id].end());
            break;

        case PARENT_RANDOM:
            {   //Need a block here to avoid comiler error reusing random_individual variable
                uint32_t random_individual = (*random_number_generator)() * population_size;
                parent.assign(population[random_individual].begin(), population[random_individual].end());
            }
            break;

        case PARENT_CURRENT_TO_BEST:
            for (uint32_t i = 0; i < number_parameters; i++) {
                parent[i] = parent_scaling_factor * (population[global_best_id][i] - population[id][i]);
            }
            break;

        case PARENT_CURRENT_TO_RANDOM:
            {   //Need a block here to avoid comiler error reusing random_individual variable
                uint32_t random_individual = (*random_number_generator)() * population_size;
                for (uint32_t i = 0; i < number_parameters; i++) {
                    parent[i] = parent_scaling_factor * (population[random_individual][i] - population[id][i]);
                }
            }
            break;

        default:
            std::stringstream oss;
            oss << "ERROR [file: " << __FILE__ << ", line: " << __LINE__ << "]: unknown parent selection type (" << parent_selection << ")";
            throw oss.str();
            break;
    }

    /**
     *  Calculate the differentials.
     */
    vector<double> differential(number_parameters, 0);

    uint32_t random_individual1;
    uint32_t random_individual2; 
    for (uint32_t i = 0; i < number_pairs * 2; i++) {
        random_individual1 = (*random_number_generator)() * population_size;
        random_individual2 = (*random_number_generator)() * population_size;

        if (directional) { //Used for directional recombination (although that part is not the recombination step)
            if (fitnesses[random_individual2] > fitnesses[random_individual1]) {
                uint32_t temp = random_individual1;
                random_individual1 = random_individual2;
                random_individual2 = temp;
            }
        }

        for (uint32_t j = 0; j < number_parameters; j++) {
            differential[j] = population[random_individual1][j] - population[random_individual2][j];
        }
    }

    for (uint32_t i = 0; i < number_parameters; i++) differential[i] *= differential_scaling_factor / number_pairs;

    /**
     *  Perform the recombination.
     */
//    cout << "differential: " << vector_to_string(differential) << " parent: " << vector_to_string(parent) << endl;

    for (uint32_t i = 0; i < number_parameters; i++) parent[i] += differential[i];

    switch (recombination_selection) {
        case RECOMBINATION_BINARY:
            Recombination::binary_recombination(population[id], parent, crossover_rate, parameters, random_number_generator);
            Recombination::bound_parameters(min_bound, max_bound, parameters);
            break;

        case RECOMBINATION_EXPONENTIAL:
            Recombination::exponential_recombination(population[id], parent, crossover_rate, parameters, random_number_generator);
            Recombination::bound_parameters(min_bound, max_bound, parameters);
            break;

        case RECOMBINATION_SUM:
            for (uint32_t i = 0; i < number_parameters; i++) parameters[i] = population[id][i] + parent[i];
            Recombination::bound_parameters(min_bound, max_bound, parameters);
            break;

        case RECOMBINATION_NONE:
            Recombination::bound_parameters(min_bound, max_bound, parent);
            parameters.assign(parent.begin(), parent.end());
            break;

        default:
            std::stringstream oss;
            oss << "ERROR [file: " << __FILE__ << ", line: " << __LINE__ << "]: unknown recombination type (" << recombination_selection << ")";
            throw oss.str();
            break;
    }

//    cout << "new individual: " << vector_to_string(parameters) << endl;
//    cout << "population[" << id << "]: " << vector_to_string(population[id]) << endl;
    individuals_created++;
}


bool
DifferentialEvolution::insert_individual(uint32_t id, const std::vector<double> &parameters, double fitness, uint32_t seed) throw (string) {
    bool modified = false;
    if (fitnesses[id] < fitness) {
        if (fitnesses[id] == -numeric_limits<double>::max()) initialized_individuals++;

        fitnesses[id] = fitness;
        population[id].assign(parameters.begin(), parameters.end());

        cout.precision(15);
//        cout <<  current_iteration << ":" << id << " - LOCAL: " << fitness << " " << vector_to_string(parameters) << endl;

        if (global_best_fitness < fitness) {
            global_best_id = id;
            global_best_fitness = fitness;

            if (log_file == NULL) {
                cout.precision(15);
                cout <<  current_iteration << ":" << id << " - GLOBAL: " << global_best_fitness << " " << vector_to_string(parameters) << endl;
                cout <<  current_iteration << ":" << id << " - GLOBAL: " << global_best_fitness << " " << vector_to_string(parameters) << endl;
            }
        }

        if (log_file != NULL) {
            double best, average, median, worst;
            calculate_fitness_statistics(fitnesses, best, average, median, worst);
            (*log_file) << individuals_reported << " -- b: " << best << ", a: " << average << ", m: " << median << ", w: " << worst << endl;
        } 

        modified = true;
    }
    individuals_reported++;
    return modified;
}


bool
DifferentialEvolution::would_insert(uint32_t id, double fitness) {
    return fitnesses[id] < fitness;
}
 
/**
 *  The following method is for synchronous optimization and is purely virtual
 */
void
DifferentialEvolution::iterate(double (*objective_function)(const std::vector<double> &)) throw (string) {
    cout << "Initialized differential evolution. " << endl;
    cout << "   maximum_iterations:          " << maximum_iterations << endl;
    cout << "   current_iteration:           " << current_iteration << endl;
    cout << "   number_pairs:                " << number_pairs << endl;
    cout << "   parent_selection:            " << parent_selection << endl;
    cout << "   recombination_selection:     " << recombination_selection << endl;
    cout << "   parent_scaling_factor:       " << parent_scaling_factor << endl;
    cout << "   differential_scaling_factor: " << differential_scaling_factor << endl;
    cout << "   crossover_rate:              " << crossover_rate << endl;
    cout << "   directional:                 " << directional << endl;

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
DifferentialEvolution::iterate(double (*objective_function)(const std::vector<double> &, const uint32_t)) throw (string) {
    cout << "Initialized differential evolution. " << endl;
    cout << "   maximum_iterations:          " << maximum_iterations << endl;
    cout << "   current_iteration:           " << current_iteration << endl;
    cout << "   number_pairs:                " << number_pairs << endl;
    cout << "   parent_selection:            " << parent_selection << endl;
    cout << "   recombination_selection:     " << recombination_selection << endl;
    cout << "   parent_scaling_factor:       " << parent_scaling_factor << endl;
    cout << "   differential_scaling_factor: " << differential_scaling_factor << endl;
    cout << "   crossover_rate:              " << crossover_rate << endl;
    cout << "   directional:                 " << directional << endl;

    uint32_t id;
    uint32_t seed;
    vector<double> parameters(number_parameters, 0);

    while (maximum_iterations == 0 || current_iteration < maximum_iterations) {
        for (uint32_t i = 0; i < population_size; i++) {
            new_individual(id, parameters, seed);
            double fitness = objective_function(parameters, seed);
            insert_individual(id, parameters, fitness, seed);
        }

        current_iteration++;
    }
}

void
DifferentialEvolution::get_individuals(std::vector<Individual> &individuals) {
    individuals.clear();
    for (uint32_t i = 0; i < population_size; i++) {
        individuals.push_back(Individual(i, fitnesses[i], population[i], ""));
    }
}
