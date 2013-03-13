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

#include "undvc_common/arguments.hxx"

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

    double start_time = MPI_Wtime();
    double probe_time = 0, start_probe_time;

    while (true) {
        //Wait on a message from any worker

        start_probe_time = MPI_Wtime();
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        probe_time += MPI_Wtime() - start_probe_time;

        int n_requested_individuals;
        int source = status.MPI_SOURCE;
        int tag = status.MPI_TAG;

        double fitness = 0;

        if (tag == REQUEST_INDIVIDUALS_TAG) {
            MPI_Recv(&n_requested_individuals, 1, MPI_INT, source, REQUEST_INDIVIDUALS_TAG, MPI_COMM_WORLD, &status);
            cout << "[master      ] request from worker " << source << " for " << n_requested_individuals << " individuals, with tag: " << tag << ". " << endl;

            for (int i = 0; i < n_requested_individuals; i++) {
                vector<T> new_individual(ea->get_number_parameters(), 0);
                ea->new_individual(individual_position, new_individual);

                /*
                cout << "[master      ] sending individual: [";
                for (int i = 0; i < number_parameters; i++) {
                    cout << " " << new_individual[i];
                }
                cout << "]" << endl;
                */

                MPI_Send(&new_individual[0], number_parameters, MPI_DATATYPE, source, REQUEST_INDIVIDUALS_TAG, MPI_COMM_WORLD);
                MPI_Send(&individual_position, 1, MPI_INT, source, REQUEST_INDIVIDUALS_TAG, MPI_COMM_WORLD);
            }

        } else if (tag == REPORT_FITNESS_TAG) {

            MPI_Recv(&fitness, 1, MPI_DOUBLE, source, REPORT_FITNESS_TAG, MPI_COMM_WORLD, &status);
            MPI_Recv(individual, number_parameters, MPI_DATATYPE, source, REPORT_FITNESS_TAG, MPI_COMM_WORLD, &status);
            MPI_Recv(&individual_position, 1, MPI_INT, source, REPORT_FITNESS_TAG, MPI_COMM_WORLD, &status);

            /*
            cout << "[master      ] received fitness: " << fitness << endl;
            cout << "[master      ] received individual: [";
            for (int i = 0; i < number_parameters; i++) {
                cout << " " << individual[i];
            }
            cout << "]" << endl;
            */

            vector<T> received_individual(individual, individual + ea->get_number_parameters());
            ea->insert_individual(individual_position, received_individual, fitness);

            vector<T> new_individual(ea->get_number_parameters(), 0);
            ea->new_individual(individual_position, new_individual);

            MPI_Send(&new_individual[0], number_parameters, MPI_DATATYPE, source, REQUEST_INDIVIDUALS_TAG, MPI_COMM_WORLD);
            MPI_Send(&individual_position, 1, MPI_INT, source, REQUEST_INDIVIDUALS_TAG, MPI_COMM_WORLD);
            //            cout << "[master       ] sent individual to " << source << endl;
        } else {
            cerr << "[master      ] Unknown tag '" << tag << "' received from MPI_Probe on file '" << __FILE__ << "', line " << __LINE__ << endl;
            MPI_Finalize();
            exit(1);
        }

        if (!ea->is_running()) {
            cout << endl;
            cout << "[master      ] completed in " << (MPI_Wtime() - start_time) << " seconds, time spent probing: " << probe_time << " seconds." << endl;
            cout << endl;

            //The termination conditions for the search have been met
            for (int i = 1; i < max_rank; i++) {
//                cout << "[master      ] sending terminate to process: " << i << endl;
                //Just send an int we don't need any contents -- the terminate tag will make the worker quit
                individual_position = 0;    
                MPI_Send(&individual_position, 1, MPI_INT, i, TERMINATE_TAG, MPI_COMM_WORLD);

                double times[7];
                MPI_Recv(times, 7, MPI_DOUBLE, i, TERMINATE_TAG, MPI_COMM_WORLD, &status);
                cout << "[worker " << setw(5) << i << "] terminated -- n: " << times[0] << ", pt: " << times[1] << ", min_pt: " << times[2] << ", max_pt: " << times[3] << ", ct: " << times[4] << ", min_ct: " << times[5] << ", max_ct: " << times[6] << endl;

//                cout << "[worker " << setw(5) << rank << "] processing_time: " << processing_time << ", communication_time: " << communication_time << endl;
//                cout << "[worker " << setw(5) << rank << "] min_processing_time: " << min_processing_time << ", max_processing_time: " << max_processing_time << endl;
//                cout << "[worker " << setw(5) << rank << "] min_communication_time: " << min_communication_time << ", max_communication_time: " << max_communication_time << endl;
            }
            return;
        }
    }
}

template void master<ParticleSwarmMPI, double>(ParticleSwarmMPI *ea);
template void master<GeneticAlgorithmMPI, int>(GeneticAlgorithmMPI *ea);

template <typename T>
void worker(double (*objective_function)(const std::vector<T> &),
            int number_parameters,
            int max_queue_size
           ) {

    MPI_Status status;
    int flag;
    int tag;
    int individual_position;
    queue< int > individuals_position_queue;
    queue< vector<T>* > individuals_queue;
    int rank;
    T individual[number_parameters];

    mpi_type_wrapper<T> parameter_type;
    MPI_Datatype MPI_DATATYPE = parameter_type.mpi_type;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /**
     *  Request enough work to fill the workers queue of individuals
     */
    MPI_Send(&max_queue_size, 1, MPI_INT, 0 /*master is rank 0*/, REQUEST_INDIVIDUALS_TAG, MPI_COMM_WORLD);

    //Fill up the initial queue
    for (int i = 0; i < max_queue_size; i++) {
        MPI_Recv(individual, number_parameters, MPI_DATATYPE, 0 /*master is rank 0*/, REQUEST_INDIVIDUALS_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&individual_position, 1, MPI_INT, 0 /*master is rank 0*/, REQUEST_INDIVIDUALS_TAG, MPI_COMM_WORLD, &status);

        vector<T> *new_individual = new vector<T>(individual, individual + number_parameters);
        individuals_queue.push(new_individual);
        individuals_position_queue.push(individual_position);
    }

    long processing_jobs = 0;
    double communication_time = 0, communication_start;
    double processing_time = 0, processing_start;
    double min_processing_time = numeric_limits<double>::max(), max_processing_time = 0;
    double min_communication_time = numeric_limits<double>::max(), max_communication_time = 0;
    double current_communication_time, current_processing_time;

    //Loop forever calculating individual fitness
    communication_start = MPI_Wtime();
    while (true) {
        if (individuals_queue.size() == 0) {
            //The queue is empty, block waiting for a message from the master
            MPI_Probe(0 /*master is rank 0*/, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            flag = 1;
        } else {
            //The queue is not empty, check to see if there's an incoming message from the master
            MPI_Iprobe(0 /*master is rank 0*/, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
        }

        while (flag) {
            tag = status.MPI_TAG;
            if (tag == REQUEST_INDIVIDUALS_TAG) {
                //there's an incoming individual, receive it.
                MPI_Recv(individual, number_parameters, MPI_DATATYPE, 0 /*master is rank 0*/, REQUEST_INDIVIDUALS_TAG, MPI_COMM_WORLD, &status);
                MPI_Recv(&individual_position, 1, MPI_INT, 0 /*master is rank 0*/, REQUEST_INDIVIDUALS_TAG, MPI_COMM_WORLD, &status);

                vector<T> *new_individual = new vector<T>(individual, individual + number_parameters);
                individuals_queue.push(new_individual);
                individuals_position_queue.push(individual_position);
            } else if (tag == TERMINATE_TAG) {
                MPI_Recv(&individual_position, 1, MPI_INT, 0 /*master is rank 0*/, TERMINATE_TAG, MPI_COMM_WORLD, &status);
//                cerr << "[worker " << setw(5) << rank << "] Received terminate tag: " << tag << endl;

                //Clear out the queue
                while (individuals_queue.size() > 0) {
                    vector<T> *current_individual = individuals_queue.front();
                    individuals_queue.pop();
                    delete current_individual;
                }

                double times[] = { processing_jobs, processing_time, min_processing_time, max_processing_time, communication_time, min_communication_time, max_communication_time };

                MPI_Send(times, 7, MPI_DOUBLE, 0 /*master is rank 0*/, TERMINATE_TAG, MPI_COMM_WORLD);
//                cout << "[worker " << setw(5) << rank << "] processing_time: " << processing_time << ", communication_time: " << communication_time << endl;
//                cout << "[worker " << setw(5) << rank << "] min_processing_time: " << min_processing_time << ", max_processing_time: " << max_processing_time << endl;
//                cout << "[worker " << setw(5) << rank << "] min_communication_time: " << min_communication_time << ", max_communication_time: " << max_communication_time << endl;

                return;
            } else {
                cerr << "[worker " << setw(5) << rank << "] Received unknown tag: " << tag << endl;
                return;
            }

            MPI_Iprobe(0 /*master is rank 0*/, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
        }

        //Pop the next individual off the queue
        vector<T> *current_individual = individuals_queue.front();
        individuals_queue.pop();

        int current_individual_position = individuals_position_queue.front();
        individuals_position_queue.pop();

        current_communication_time = MPI_Wtime() - communication_start;
        communication_time += current_communication_time;

        //calculate the fitness of the head of the individual queue
        processing_start = MPI_Wtime();
        double fitness = objective_function(*current_individual);
        current_processing_time = MPI_Wtime() - processing_start;
        processing_time += current_processing_time;
        processing_jobs++;

        if (current_processing_time < min_processing_time) min_processing_time = current_processing_time;
        if (current_processing_time > max_processing_time) max_processing_time = current_processing_time;
        if (current_communication_time < min_communication_time) min_communication_time = current_communication_time;
        if (current_communication_time > max_communication_time) max_communication_time = current_communication_time;

        communication_start = MPI_Wtime();

        //Send the fitness and the individual back to the master
        MPI_Send(&fitness, 1, MPI_DOUBLE, 0 /*master is rank 0*/, REPORT_FITNESS_TAG, MPI_COMM_WORLD);
        MPI_Send(&(*current_individual)[0], number_parameters, MPI_DATATYPE, 0 /*master is rank 0 */, REPORT_FITNESS_TAG, MPI_COMM_WORLD);
        MPI_Send(&current_individual_position, 1, MPI_INT, 0 /*master is rank 0 */, REPORT_FITNESS_TAG, MPI_COMM_WORLD);

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