#ifndef TAO_MPI_GENETIC_ALGORITHM_H
#define TAO_MPI_GENETIC_ALGORITHM_H

#include <vector>

#include "asynchronous_algorithms/asynchronous_genetic_search.hxx"

using std::vector;

class GeneticAlgorithmMPI : public GeneticAlgorithm {
    private:
        int max_queue_size;
    public:
        GeneticAlgorithmMPI(const vector<string> &arguments,
                            int encoding_length,
                            random_encoding_type random_encoding,
                            mutate_type mutate,
                            crossover_type crossover);

        void go(objective_function_type objective_function);
};

#endif
