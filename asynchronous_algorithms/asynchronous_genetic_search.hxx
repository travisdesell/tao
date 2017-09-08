#ifndef TAO_ASYNCHRONOUS_GENETIC_SEARCH_H
#define TAO_ASYNCHRONOUS_GENETIC_SEARCH_H

#include <queue>
#include <random>
using std::mt19937;

#include <vector>

using namespace std;

typedef double (*objective_function_type)(const vector<int> &);
typedef vector<int> (*random_encoding_type)();
typedef vector<int> (*mutate_type)(const vector<int> &);
typedef vector<int> (*crossover_type)(const vector<int> &, const vector<int> &);

class GeneticAlgorithmIndividual {
    public:
        const double fitness;
        const vector<int> encoding;

        GeneticAlgorithmIndividual(double _fitness, const vector<int> &_encoding);
};

class GeneticAlgorithm {
    private:
        int individuals_reported;
        int individuals_created;
        int maximum_created;
        int maximum_reported;
        int current_iteration;
        int maximum_iteration;

        bool too_many_duplicates;

        double start_time;

        vector<GeneticAlgorithmIndividual*> population;

        mt19937 random_number_generator;
        uniform_real_distribution<double> random_0_1;

        int find_insert_position(double fitness, int min_position, int current_position, int max_position);

        bool is_duplicate(const vector<int> &new_individual);

    protected:
        double mutation_rate;
        double crossover_rate;

        int population_size;
        int encoding_length;

        random_encoding_type random_encoding;
        mutate_type mutate;
        crossover_type crossover;

    public:
        void (*print_statistics)(const std::vector<int> &);

        int get_current_iteration() {
            return current_iteration;
        }

        double get_global_best_fitness() {
            return population[0]->fitness;
        }

        vector<int> get_global_best() {
            return vector<int>(population[0]->encoding);
        }

        int get_number_parameters()   { return encoding_length; }

        GeneticAlgorithm(const vector<string> &arguments,
                         int _encoding_length,
                         random_encoding_type _random_encoding,
                         mutate_type _mutate,
                         crossover_type _crossover);


        void new_individual(uint32_t &individual_position, vector<int> &individual);
        void insert_individual(uint32_t individual_position, const int* encoding, double fitness);
        void insert_individual(uint32_t individual_position, const vector<int> &encoding, double fitness);
        
        bool is_running();

        void set_print_statistics(void (*_print_statistics)(const std::vector<int> &));
};


#endif
