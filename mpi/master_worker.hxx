#ifndef TAO_MPI_MASTER_WORKER_H
#define TAO_MPI_MASTER_WORKER_H

#include <vector>

using std::vector;

#define REQUEST_INDIVIDUALS_TAG 0
#define REPORT_FITNESS_TAG 1000
#define TERMINATE_TAG 2000


template<typename EvolutionaryAlgorithmsType, typename T>
void master(EvolutionaryAlgorithmsType *ea);

template<typename T>
void worker(double (*objective_function)(const std::vector<T> &),
            int number_parameters,
            int max_queue_size);

template<typename T>
void set_print_statistics(double (*_print_statistics)(const std::vector<T> &));

#endif
