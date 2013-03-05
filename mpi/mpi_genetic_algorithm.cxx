#include <vector>
#include <iostream>
#include <iomanip>
#include <queue>

#include "mpi.h"

#include "asynchronous_algorithms/asynchronous_genetic_search.hxx"
#include "mpi/mpi_genetic_algorithm.hxx"

using namespace std;

template <typename T>
void master(int encoding_length) {
    MPI_Status status;
    int individual[encoding_length];

    while (true) {
        //Wait on a message from any worker
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        int n_requested_individuals;
        int source = status.MPI_SOURCE;
        int tag = status.MPI_TAG;
        if (tag == REQUEST_INDIVIDUALS_TAG) {
            MPI_Recv(&n_requested_individuals, 1, MPI_INT, source, REQUEST_INDIVIDUALS_TAG, MPI_COMM_WORLD, &status);
            cout << "worker " << source << " requested: " << n_requested_individuals << " individuals, with a tag: " << tag << ". " << endl;

            for (int i = 0; i < n_requested_individuals; i++) {
                vector<int> new_individual = random_encoding();

                MPI_Send(&new_individual[0], encoding_length, MPI_INT, source, REQUEST_INDIVIDUALS_TAG, MPI_COMM_WORLD);
                cout << "[master      ] sent individual to " << source << endl;
            }

        } else if (tag == REPORT_FITNESS_TAG) {
            double fitness;
            MPI_Recv(&fitness, 1, MPI_DOUBLE, source, REPORT_FITNESS_TAG, MPI_COMM_WORLD, &status);
            MPI_Recv(individual, encoding_length, MPI_INT, source, REPORT_FITNESS_TAG, MPI_COMM_WORLD, &status);

            cout << "[master      ] recv fitness " << setw(10) << fitness << " from " << setw(4) << source << " [";
            for (int i = 0; i < encoding_length; i++) {
                cout << " " << individual[i];
            }
            cout << "]" << endl;

            vector<int> new_individual = random_encoding();

            MPI_Send(&new_individual[0], encoding_length, MPI_INT, source, REQUEST_INDIVIDUALS_TAG, MPI_COMM_WORLD);
//            cout << "[master      ] sent individual to " << source << endl;
        } else {
            cerr << "Unknown tag '" << tag << "' received from MPI_Probe on file '" << __FILE__ << "', line " << __LINE__ << endl;
            MPI_Finalize();
            exit(1);
        }
    }
}

void worker(int rank, objective_function *f, int encoding_length, int max_queue_size) {
    MPI_Status status;
    int tag;
    int individual[encoding_length];
    queue< vector<int>* > individuals_queue;

    /**
     *  Request enough work to fill the workers queue of individuals
     */
    MPI_Send(&max_queue_size, 1, MPI_INT, 0 /*master is rank 0*/, REQUEST_INDIVIDUALS_TAG, MPI_COMM_WORLD);

    //Fill up the initial queue
    for (int i = 0; i < max_queue_size; i++) {
        MPI_Recv(individual, encoding_length, MPI_INT, 0 /*master is rank 0*/, REQUEST_INDIVIDUALS_TAG, MPI_COMM_WORLD, &status);

        vector<int> *new_individual = new vector<int>(individual, individual + encoding_length);
        individuals_queue.push(new_individual);
        cout << "[worker " << setw(5) << rank << "] received an initial individual, queue size: " << individuals_queue.size() << endl;
    }

    //Loop forever calculating individual fitness
    while (true) {
        if (individuals_queue.size() == 0) {
            //The queue is empty, block waiting for a message from the master
            MPI_Probe(0 /*master is rank 0*/, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            tag = status.MPI_TAG;
            MPI_Recv(individual, encoding_length, MPI_INT, 0 /*master is rank 0*/, tag, MPI_COMM_WORLD, &status);

            vector<int> *new_individual = new vector<int>(individual, individual + encoding_length);
            individuals_queue.push(new_individual);
            cout << "[worker " << setw(5) << rank << "] received an individual (tag " << tag << "), queue size: " << individuals_queue.size() << endl;
        } 

        //The queue is not empty, check to see if there's an incoming message from the master
        int flag;
        MPI_Iprobe(0 /*master is rank 0*/, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
//        cout << "[worker " << setw(5) << rank << "] probe returned flag: " << flag << endl;

        while (flag) {
            tag = status.MPI_TAG;
            //there's an incoming individual, receive it.
            MPI_Recv(individual, encoding_length, MPI_INT, 0 /*master is rank 0*/, tag, MPI_COMM_WORLD, &status);

            vector<int> *new_individual = new vector<int>(individual, individual + encoding_length);
            individuals_queue.push(new_individual);
            cout << "[worker " << setw(5) << rank << "] received an individual (tag " << tag << "), queue size: " << individuals_queue.size() << endl;

            MPI_Iprobe(0 /*master is rank 0*/, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
        }

        //Pop the next individual off the queue
        vector<int> *current_individual = individuals_queue.front();
        individuals_queue.pop();

        /*
        cout << "[worker " << setw(5) << rank << "] calculating fitness on           [";
        for (int j = 0; j < encoding_length; j++) {
            cout << " " << current_individual->at(j);
        }
        cout << "]" << endl;
        */

        //calculate the fitness of the head of the individual queue
        double fitness = f(*current_individual);

        /*
        cout << "[worker " << setw(5) << rank << "] calc fitness " << setw(10) << fitness << "           [";
        for (int j = 0; j < encoding_length; j++) {
            cout << " " << current_individual->at(j);
        }
        cout << "]" << endl;
        */

        //Send the fitness and the individual back to the master
        MPI_Send(&fitness, 1, MPI_DOUBLE, 0 /*master is rank 0*/, REPORT_FITNESS_TAG, MPI_COMM_WORLD);
        MPI_Send(&(*current_individual)[0], encoding_length, MPI_INT, 0 /*master is rank 0 */, REPORT_FITNESS_TAG, MPI_COMM_WORLD);

        //Delete the popped individual
        delete current_individual;
    }
}


