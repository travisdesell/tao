#ifndef TAO_MPI_ACO_H
#define TAO_MPI_ACO_H

#include <vector>
using std::vector;

#include "asynchronous_algorithms/ant_colony_optimization.hxx"

#include "neural_networks/edge.hxx"


void ant_colony_optimization_mpi(int maximum_iterations, AntColony &ant_colony, double (*objective_function)(vector<Edge> &edges, vector<Edge> &recurrent_edges));

#endif
