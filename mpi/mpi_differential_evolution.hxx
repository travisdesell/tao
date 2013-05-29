#ifndef TAO_MPI_DIFFERENTIAL_EVOLUTION_H
#define TAO_MPI_DIFFERENTIAL_EVOLUTION_H

#include <vector>

#include "asynchronous_algorithms/differential_evolution.hxx"

using std::vector;

class DifferentialEvolutionMPI : public DifferentialEvolution {
    private:
        int max_queue_size;

    public:
        DifferentialEvolutionMPI(const std::vector<double> &min_bound,      /* min bound is copied into the search */
                         const std::vector<double> &max_bound,      /* max bound is copied into the search */
                         const vector<string> &arguments
                        );

        void go(double (*objective_function)(const std::vector<double> &));

#ifdef CUDA
        void go(double (*cpu_objective_function)(const std::vector<double> &),
                double (*gpu_objective_function)(const std::vector<double> &),
                int *device_assignments);
#endif
};

#endif
