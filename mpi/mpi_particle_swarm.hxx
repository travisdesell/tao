#ifndef TAO_MPI_PARTICLE_SWARM_H
#define TAO_MPI_PARTICLE_SWARM_H

#include <vector>

#include "asynchronous_algorithms/particle_swarm.hxx"

using std::vector;

class ParticleSwarmMPI : public ParticleSwarm {
    private:
        int max_queue_size;

    public:
        ParticleSwarmMPI(const std::vector<double> &min_bound,      /* min bound is copied into the search */
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
