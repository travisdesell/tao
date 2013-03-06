#ifndef TAO_MPI_GENETIC_ALGORITHM_H
#define TAO_MPI_GENETIC_ALGORITHM_H

#include <vector>

#include "asynchronous_algorithms/asynchronous_genetic_search.hxx"

using std::vector;

#define REQUEST_INDIVIDUALS_TAG 0
#define REPORT_FITNESS_TAG 1000

class GeneticAlgorithmMPI : GeneticAlgorithm {
    private:
        int max_queue_size;
        int rank;

        void master();
        void worker(objective_function_type objective_function);

    public:
        GeneticAlgorithmMPI(const vector<string> &arguments,
                            int encoding_length,
                            random_encoding_type random_encoding,
                            mutate_type mutate,
                            crossover_type crossover);

        void go(objective_function_type objective_function);
};

#endif
