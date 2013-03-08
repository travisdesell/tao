#ifndef TAO_MPI_GENETIC_ALGORITHM_H
#define TAO_MPI_GENETIC_ALGORITHM_H

#include <vector>

#include "asynchronous_algorithms/particle_swarm.hxx"

using std::vector;

#define REQUEST_INDIVIDUALS_TAG 0
#define REPORT_FITNESS_TAG 1000

class ParticleSwarmMPI : public ParticleSwarm {
    private:
        int max_queue_size;
        int rank;

        void master();
        void worker(double (*objective_function)(const std::vector<double> &));

    public:
        ParticleSwarmMPI(const std::vector<double> &min_bound,            /* min bound is copied into the search */
                         const std::vector<double> &max_bound,            /* max bound is copied into the search */
                         const vector<string> &arguments
                        );

        void go(double (*objective_function)(const std::vector<double> &));
};

#endif
