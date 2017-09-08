#include <vector>
#include <queue>
#include <iostream>
#include <iomanip>
#include <limits>

#include "asynchronous_algorithms/asynchronous_genetic_search.hxx"

#include "util/arguments.hxx"

using namespace std;

void
GeneticAlgorithm::set_print_statistics(void (*_print_statistics)(const std::vector<int> &)) {
    print_statistics = _print_statistics;
}


bool compare_individuals(const GeneticAlgorithmIndividual *i1, const GeneticAlgorithmIndividual *i2) {
    return i1->fitness > i2->fitness;
}

GeneticAlgorithmIndividual::GeneticAlgorithmIndividual(double _fitness, const vector<int> &_encoding) : fitness(_fitness), encoding(_encoding) {
}

GeneticAlgorithm::GeneticAlgorithm(const vector<string> &arguments,
                 int _encoding_length,
                 random_encoding_type _random_encoding,
                 mutate_type _mutate,
                 crossover_type _crossover) : encoding_length(_encoding_length), random_encoding(_random_encoding), mutate(_mutate), crossover(_crossover) {

    if (!get_argument(arguments, "--maximum_reported", false, maximum_reported)) {
        cerr << "Argument '--maximum_reported <I>' not specified, could run forever.Hit control-C to quit." << endl;
        maximum_reported = 0;
    }

    if (!get_argument(arguments, "--maximum_created", false, maximum_created)) {
        cerr << "Argument '--maximum_created <I>' not specified, could run forever.Hit control-C to quit." << endl;
        maximum_created = 0;
    }

    if (!get_argument(arguments, "--population_size", false, population_size)) {
        cerr << "Argument '--population_size <I>' not found, using default of 50." << endl;
        population_size = 50;
    }

    if (!get_argument(arguments, "--mutation_rate", false, mutation_rate)) {
        cerr << "Argument '--mutation_rate <F>' not found, using default of 0.5." << endl;
        mutation_rate = 0.5;
    }

    if (!get_argument(arguments, "--crossover_rate", false, crossover_rate)) {
        cerr << "Argument '--crossover_rate <F>' not found, using default of 0.5." << endl;
        crossover_rate = 0.5;
    }

    if (mutation_rate + crossover_rate != 1.0) {
        cerr << "WARNING: mutation_rate (" << mutation_rate << ") + crossover_rate (" << crossover_rate << ") != 1.0" << endl;
        mutation_rate = 1.0 - crossover_rate;
        cerr << "\tSetting mutation_rate = 1.0 - crossover_rate = " << mutation_rate << endl;
    }

    random_number_generator = mt19937(time(0));
    random_0_1 = uniform_real_distribution<double>(0, 1.0);
    //random_number_generator = new variate_generator< mt19937, uniform_real<> >( mt19937( time(0)), uniform_real<>(0.0, 1.0));

    individuals_created = 0;
    individuals_reported = 0;
    current_iteration = 0;

    too_many_duplicates = false;

    start_time = time(NULL);
    print_statistics = NULL;
}

bool GeneticAlgorithm::is_duplicate(const vector<int> &encoding) {
    //Make sure the individual generated isn't in the population
    bool same;
    for (unsigned int i = 0; i < population.size(); i++) {
        same = true;
        for (unsigned int j = 0; j < encoding.size(); j++) {
            if (population[i]->encoding[j] != encoding[j]) {
                same = false;
                break;
            }
        }
        if (same) return true;
    }
    return false;
}

void GeneticAlgorithm::new_individual(uint32_t &individual_position, vector<int> &individual) {
    individuals_created++;
    if (individuals_created % population_size == 0) current_iteration++;

    individual_position = 0;    //we can ignore the position

    int count = 0;
    do {
        if (population.size() < population_size) {
            individual = random_encoding();
        } else {
            if (random_0_1(random_number_generator) < mutation_rate) {
                int position = random_0_1(random_number_generator) * population_size;
                individual = mutate( population[position]->encoding );
            } else {
                int position1 = random_0_1(random_number_generator) * population_size;
                int position2 = random_0_1(random_number_generator) * (population_size - 1);

                if (position2 >= position1) position2++;

                individual = crossover(population[position1]->encoding, population[position2]->encoding);
            }
        }
        count++;
    } while (is_duplicate(individual) && count < 500); //try again on duplicates

    if (count >= 500) too_many_duplicates = true;

}

void GeneticAlgorithm::insert_individual(uint32_t individual_position, const int* encoding, double fitness) {
    vector<int> encoding_v(encoding, encoding + encoding_length);
    insert_individual(individual_position, encoding_v, fitness);
}

void GeneticAlgorithm::insert_individual(uint32_t individual_position, const vector<int> &encoding, double fitness) {
    individuals_reported++;

    //This fitness is worse than anything in the population, discard it
    if (population.size() == population_size && fitness < population[population_size - 1]->fitness) {
        /*
        cout << "[master     ] " << individuals_reported << "/" << individuals_created << " -- discarding fitness " << fitness << " [";
        for (int i = 0; i < encoding_length; i++) {
            cout << " " << encoding[i];
        }
        cout << "]" << endl;
        */
        return;
    }

    //Don't insert a duplicate encoding
    if (is_duplicate(encoding)) {
//        cout << "[master     ] received duplicate individual, discarding." << endl;
        return;
    }

    GeneticAlgorithmIndividual *ga_individual = new GeneticAlgorithmIndividual(fitness, encoding);

    //sorting a vector is slower but priority queues aren't working right
    population.push_back(ga_individual);
    sort(population.begin(), population.end(), compare_individuals);

    if (population.size() > population_size) {
        ga_individual = population.back();
        population.pop_back();

        delete ga_individual;
    }

    //Found a new best fitness
    if (fitness == population[0]->fitness) {
        long run_time = time(NULL) - start_time;
        cout << setw(10) << run_time << setw(7) << individuals_reported << "/" << setw(7) << individuals_created;
        cout << "[" << setw(10) << ((double)individuals_reported / (double)run_time) << "] ";
        cout << "[b: " << setw(9) << population[0]->fitness << ", m: " << setw(9) << population[population.size() / 2]->fitness << ", w: " << setw(9) << population[population.size() - 1]->fitness << "] ";
        cout << setw(20) << "new best fitness   " << setw(9) << fitness << " [";

        for (int i = 0; i < encoding_length; i++) {
            cout << " " << encoding[i];
        }
        cout << "]" << endl;
    }


    /*
    cout << "population fitnesses:" << endl;
    for (int i = 0; i < population.size(); i++) {
        cout << "\t[" << setw(4) << i << "] " << population[i]->fitness << endl;
    }
    */
}

bool GeneticAlgorithm::is_running() {
    return (maximum_created <= 0 || individuals_created < maximum_created) &&
           (maximum_reported <= 0 || individuals_reported < maximum_reported) &&
           !too_many_duplicates;
}
