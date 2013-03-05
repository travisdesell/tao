#ifndef TAO_MPI_GENETIC_ALGORITHM_H
#define TAO_MPI_GENETIC_ALGORITHM_H

#include <vector>

using std::vector;

#define REQUEST_INDIVIDUALS_TAG 0
#define REPORT_FITNESS_TAG 1000

typedef double (*objective_function)(const vector<int> &);

void master(int encoding_length);

void worker(int rank, objective_function f, int encoding_length, int max_queue_size);

#endif
