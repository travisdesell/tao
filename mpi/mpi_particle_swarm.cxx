#include <vector>
#include <iostream>
#include <iomanip>
#include <queue>

#include "mpi.h"

#include "asynchronous_algorithms/particle_swarm.hxx"

#include "mpi/master_worker.hxx"
#include "mpi/mpi_particle_swarm.hxx"

#include "undvc_common/arguments.hxx"

using namespace std;

ParticleSwarmMPI::ParticleSwarmMPI(const std::vector<double> &min_bound,            /* min bound is copied into the search */
                                   const std::vector<double> &max_bound,            /* max bound is copied into the search */
                                   const vector<string> &arguments
                                  ) : ParticleSwarm(min_bound, max_bound, arguments) {

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (!get_argument(arguments, "--max_queue_size", false, max_queue_size)) {
        if (rank == 0) {
            cout << "Argument '--max_queue_size <I>' not found, using default of 3." << endl;
        }
        max_queue_size = 3;
    }
}


void ParticleSwarmMPI::go(double (*objective_function)(const std::vector<double> &)) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        master<ParticleSwarmMPI, double>(this);
    } else {
        worker(objective_function, this->number_parameters, max_queue_size);
    }
    
    MPI_Finalize();
}

