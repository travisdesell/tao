#ifndef TAO_MPI_ACO_H
#define TAO_MPI_ACO_H

#include <vector>
using std::vector;

#include "asynchronous_algorithms/ant_colony_optimization_new.hxx"

#include "neural_networks/edge_new.hxx"


void ant_colony_optimization_mpi(int maximum_iterations, AntColonyNew &ant_colony, double (*objective_function)(vector<EdgeNew> &edges, vector<EdgeNew> &recurrent_edges));

#endif
