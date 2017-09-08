#include <vector>
#include <iostream>
#include <iomanip>
#include <queue>

#include "mpi.h"

#include "asynchronous_algorithms/asynchronous_genetic_search.hxx"

#include "mpi/master_worker.hxx"
#include "mpi/mpi_genetic_algorithm.hxx"

#include "util/arguments.hxx"

using namespace std;

GeneticAlgorithmMPI::GeneticAlgorithmMPI(const vector<string> &arguments,
                                         int encoding_length,
                                         random_encoding_type random_encoding,
                                         mutate_type mutate,
                                         crossover_type crossover) : GeneticAlgorithm(arguments, encoding_length, random_encoding, mutate, crossover) {

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (!get_argument(arguments, "--max_queue_size", false, max_queue_size)) {
        if (rank == 0) {
            cout << "Argument '--max_queue_size <I>' not found, using default of 3." << endl;
        }
        max_queue_size = 3;
    }
}


void GeneticAlgorithmMPI::go(objective_function_type objective_function) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        master<GeneticAlgorithmMPI, int>(this);
    } else {
        worker<int>(objective_function, encoding_length, max_queue_size);
    }

    MPI_Finalize();
}

