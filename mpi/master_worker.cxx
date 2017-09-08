#include <vector>
#include <iostream>
#include <iomanip>
#include <queue>
#include <limits>

#include "mpi.h"

#include "asynchronous_algorithms/particle_swarm.hxx"

#include "mpi/master_worker.hxx"
#include "mpi/mpi_genetic_algorithm.hxx"
#include "mpi/mpi_particle_swarm.hxx"
#include "mpi/mpi_differential_evolution.hxx"

#include "util/arguments.hxx"

using namespace std;


template <typename T>
struct mpi_type_wrapper {
    MPI_Datatype mpi_type;
    mpi_type_wrapper();
};

template <>
mpi_type_wrapper<int>::mpi_type_wrapper() : mpi_type(MPI_INT) {}

template <>
mpi_type_wrapper<double>::mpi_type_wrapper() : mpi_type(MPI_DOUBLE) {}

template<typename T>
void send_individual(int target, MPI_Datatype MPI_DATATYPE, const vector<T> &individual, int individual_position) {
    MPI_Send(&individual[0], individual.size(), MPI_DATATYPE, target, REQUEST_INDIVIDUALS_TAG, MPI_COMM_WORLD);
    //cout << "[master      ] sent new individual" << endl;

    MPI_Send(&individual_position, 1, MPI_INT, target, REQUEST_INDIVIDUALS_TAG, MPI_COMM_WORLD);
    //cout << "[master       ] sent individual to " << target << endl;
}


template<typename EvolutionaryAlgorithmsType, typename T>
void master(EvolutionaryAlgorithmsType *ea) {
    int max_rank, rank;
    uint32_t individual_position;
    MPI_Status status;
    int number_parameters = ea->get_number_parameters();
    T individual[number_parameters];

    mpi_type_wrapper<T> parameter_type;
    MPI_Datatype MPI_DATATYPE = parameter_type.mpi_type;

    MPI_Comm_size(MPI_COMM_WORLD, &max_rank);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int last_printed_iteration = 1;
    bool finished = false;
    double previous_best_fitness = -std::numeric_limits<double>::max();
    int unchanged_fitnesses = 0;

    double start_time = MPI_Wtime();

    for (int i = 1; i < max_rank; i++) {
        vector<T> new_individual(ea->get_number_parameters(), 0);
        ea->new_individual(individual_position, new_individual);
//        cout << "[master      ] generated new individual" << endl;

        send_individual(i, MPI_DATATYPE, new_individual, individual_position);
    }

    while (true) {
        //Wait on a message from any worker

        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        int source = status.MPI_SOURCE;

        double fitness = 0;

        //cout << "[master     ] receiving fitness." << endl;
        MPI_Recv(&fitness, 1, MPI_DOUBLE, source, REPORT_FITNESS_TAG, MPI_COMM_WORLD, &status);
        //cout << "[master     ] receiving " << number_parameters << " parameters." << endl;
        MPI_Recv(individual, number_parameters, MPI_DATATYPE, source, REPORT_FITNESS_TAG, MPI_COMM_WORLD, &status);
        //cout << "[master     ] receiving position." << endl;
        MPI_Recv(&individual_position, 1, MPI_INT, source, REPORT_FITNESS_TAG, MPI_COMM_WORLD, &status);

        //cout << "[master      ] received fitness: " << fitness << " on iteration: " << ea->get_current_iteration() << endl;
        /*
        cout << "[master      ] received individual: [";
        for (int i = 0; i < number_parameters; i++) {
            cout << " " << individual[i];
        }
        cout << "]" << endl;
        */

        vector<T> received_individual(individual, individual + ea->get_number_parameters());
        ea->insert_individual(individual_position, received_individual, fitness);

        //cout << "[master      ] inserted individual" << endl;

        vector<T> new_individual(ea->get_number_parameters(), 0);
        ea->new_individual(individual_position, new_individual);
        //cout << "[master      ] generated individual" << endl;

        send_individual(source, MPI_DATATYPE, new_individual, individual_position);

        if ((ea->get_current_iteration() % 100 == 0) && ea->get_current_iteration() != last_printed_iteration) {
            last_printed_iteration = ea->get_current_iteration();

            if (previous_best_fitness == ea->get_global_best_fitness()) {
                unchanged_fitnesses++;

                if (unchanged_fitnesses >= 50) {
                    finished = true;
                }
            } else {
                previous_best_fitness = ea->get_global_best_fitness();
                unchanged_fitnesses = 0;
            }

            cout.precision(10);
            cout << setw(10) << ea->get_current_iteration() << setw(20) << ea->get_global_best_fitness() << endl;

            if (ea->print_statistics != NULL) {
                ea->print_statistics(ea->get_global_best());
            }
            //cout <<  ea->get_current_iteration() << ":" << ea->get_global_best_fitness() << " " << vector_to_string( ea->get_global_best() ) << endl;
        }

        if (!ea->is_running() || finished) {
            cout << endl;
            cout << "[master      ] completed in " << (MPI_Wtime() - start_time) << " seconds." << endl;

            cout.precision(10);
            cout << ea->get_current_iteration() << ":" << ea->get_global_best_fitness() << " " << vector_to_string( ea->get_global_best() ) << endl;
            cout << ea->get_current_iteration() << endl;
            cout << vector_to_string( ea->get_global_best() ) << endl;
            cout << ea->get_global_best_fitness() << endl;

            break;
        }
    }

    for (int i = 1; i < max_rank; i++) {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        int source = status.MPI_SOURCE;

        double fitness = 0;


        //receive the individual the worker is sending.

        //cout << "[master     ] receiving fitness." << endl;
        MPI_Recv(&fitness, 1, MPI_DOUBLE, source, REPORT_FITNESS_TAG, MPI_COMM_WORLD, &status);
        //cout << "[master     ] receiving " << number_parameters << " parameters." << endl;
        MPI_Recv(individual, number_parameters, MPI_DATATYPE, source, REPORT_FITNESS_TAG, MPI_COMM_WORLD, &status);
        //cout << "[master     ] receiving position." << endl;
        MPI_Recv(&individual_position, 1, MPI_INT, source, REPORT_FITNESS_TAG, MPI_COMM_WORLD, &status);

        int terminate_message[1];
        terminate_message[0] = 0;

        cout << "terminating worker: " << source << endl;
        MPI_Send(terminate_message, 1, MPI_INT, source, TERMINATE_TAG, MPI_COMM_WORLD);
    }

    //MPI_Abort(MPI_COMM_WORLD, 0 /* success */);
}

template void master<DifferentialEvolutionMPI, double>(DifferentialEvolutionMPI *ea);
template void master<ParticleSwarmMPI, double>(ParticleSwarmMPI *ea);
template void master<GeneticAlgorithmMPI, int>(GeneticAlgorithmMPI *ea);

template <typename T>
void worker(double (*objective_function)(const std::vector<T> &),
            int number_parameters,
            int max_queue_size
           ) {

    MPI_Status status;
    int rank;
    uint32_t individual_position;
    T individual[number_parameters];

    mpi_type_wrapper<T> parameter_type;
    MPI_Datatype MPI_DATATYPE = parameter_type.mpi_type;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /**
     *  Request enough work to fill the workers queue of individuals
     */
    //Loop forever calculating individual fitness
    while (true) {
        //cout << "[worker " << setw(5) << rank << "] waiting to receive individual" << endl;

        MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if (status.MPI_TAG == TERMINATE_TAG) {
            int terminate_message[1];
            MPI_Recv(terminate_message, 1, MPI_INT, 0, TERMINATE_TAG, MPI_COMM_WORLD, &status);
            break;
        }

        MPI_Recv(individual, number_parameters, MPI_DATATYPE, 0 /*master is rank 0*/, REQUEST_INDIVIDUALS_TAG, MPI_COMM_WORLD, &status);

        //cout << "[worker " << setw(5) << rank << "] receiving individual" << endl;
        MPI_Recv(&individual_position, 1, MPI_INT, 0 /*master is rank 0*/, REQUEST_INDIVIDUALS_TAG, MPI_COMM_WORLD, &status);

        vector<T> *current_individual = new vector<T>(individual, individual + number_parameters);

        //cout << "[worker " << setw(5) << rank << "] starting fitness calculation" << endl;

        //calculate the fitness of the head of the individual queue
        //double processing_start = MPI_Wtime();
        double fitness = objective_function(*current_individual);
        //double current_processing_time = MPI_Wtime() - processing_start;

        //cout << "[worker " << setw(5) << rank << "] calcualted fitness: " << fitness << ", in " << current_processing_time << endl;

        //Send the fitness and the individual back to the master
        //cout << "[worker " << setw(5) << rank << "] sending fitness." << endl;
        MPI_Send(&fitness, 1, MPI_DOUBLE, 0 /*master is rank 0*/, REPORT_FITNESS_TAG, MPI_COMM_WORLD);

        //cout << "[worker " << setw(5) << rank << "] sending " << number_parameters << " parameters." << endl;
        MPI_Send(&(*current_individual)[0], number_parameters, MPI_DATATYPE, 0 /*master is rank 0 */, REPORT_FITNESS_TAG, MPI_COMM_WORLD);

        //cout << "[worker " << setw(5) << rank << "] sending position." << endl;
        MPI_Send(&individual_position, 1, MPI_INT, 0 /*master is rank 0 */, REPORT_FITNESS_TAG, MPI_COMM_WORLD);
        //cout << "[worker " << setw(5) << rank << "] sent position." << endl;

        //Delete the popped individual
        delete current_individual;
    }
}

template void worker<double>(double (*objective_function)(const std::vector<double> &),
            int number_parameters,
            int max_queue_size
           );

template void worker<int>(double (*objective_function)(const std::vector<int> &),
            int number_parameters,
            int max_queue_size
           );
