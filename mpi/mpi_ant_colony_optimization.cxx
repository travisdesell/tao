#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include <iomanip>
using std::setw;
using std::setprecision;

#include <vector>
using std::vector;

#include "mpi.h"

#include "asynchronous_algorithms/ant_colony_optimization.hxx"
#include "mpi/mpi_ant_colony_optimization.hxx"
#include "neural_networks/edge.hxx"


#define EDGES_SIZE_MSG 11
#define EDGES_ARRAY_MSG 12
#define FITNESS_MSG 12

int* edges_to_array(const vector<Edge> &edges) {
    int *edges_array = new int[edges.size() * 4];
    for (int i = 0; i < edges.size(); i++) {
        edges_array[(4 * i)    ] = edges[i].src_layer;
        edges_array[(4 * i) + 1] = edges[i].dst_layer;
        edges_array[(4 * i) + 2] = edges[i].src_node;
        edges_array[(4 * i) + 3] = edges[i].dst_node;
    }
    return edges_array;
}

void array_to_edges(vector<Edge> &edges, int array_size, int* edges_array) {
    edges.clear();
    for (int i = 0; i < array_size; i += 4) {
        edges.push_back(Edge(edges_array[i], edges_array[i+1], edges_array[i+2], edges_array[i+3]));
    }
}

double* weights_to_array(const vector<Edge> &edges) {
    double *weights_array = new double[edges.size()];
    for (int i = 0; i < edges.size(); i++) {
        weights_array[i] = edges[i].weight;
    }
    return weights_array;
}

void array_to_weights(vector<Edge> &edges, int array_size, double* weights_array) {
    for (int i = 0; i < array_size; i++) {
        edges[i].weight = weights_array[i];
    }
}




void receive_edges(int source, vector<Edge> &edges, vector<Edge> &recurrent_edges) {
    MPI_Status status;
    int array_size;

    MPI_Recv(&array_size, 1, MPI_INT, source, EDGES_SIZE_MSG, MPI_COMM_WORLD, &status);
    int *edges_array = new int[array_size];
    MPI_Recv(edges_array, array_size, MPI_INT, source, EDGES_ARRAY_MSG, MPI_COMM_WORLD, &status);
    array_to_edges(edges, array_size, edges_array);

    MPI_Recv(&array_size, 1, MPI_INT, source, EDGES_SIZE_MSG, MPI_COMM_WORLD, &status);
    double *edge_weights_array = new double[array_size];
    //if (source != 0) cout << "farmer recieved " << array_size << " edges" << endl;
    MPI_Recv(edge_weights_array, array_size, MPI_DOUBLE, source, EDGES_ARRAY_MSG, MPI_COMM_WORLD, &status);
    array_to_weights(edges, array_size, edge_weights_array);

    MPI_Recv(&array_size, 1, MPI_INT, source, EDGES_SIZE_MSG, MPI_COMM_WORLD, &status);
    int *recurrent_edges_array = new int[array_size];
    MPI_Recv(recurrent_edges_array, array_size, MPI_INT, source, EDGES_ARRAY_MSG, MPI_COMM_WORLD, &status);
    array_to_edges(recurrent_edges, array_size, recurrent_edges_array);

    MPI_Recv(&array_size, 1, MPI_INT, source, EDGES_SIZE_MSG, MPI_COMM_WORLD, &status);
    double *recurrent_edge_weights_array = new double[array_size];
    //if (source != 0) cout << "farmer recieved " << array_size << " recurrent edges" << endl;
    MPI_Recv(recurrent_edge_weights_array, array_size, MPI_DOUBLE, source, EDGES_ARRAY_MSG, MPI_COMM_WORLD, &status);
    array_to_weights(recurrent_edges, array_size, recurrent_edge_weights_array);

    delete [] edges_array;
    delete [] edge_weights_array;
    delete [] recurrent_edges_array;
    delete [] recurrent_edge_weights_array;
}

void send_edges(int target, const vector<Edge> &edges, const vector<Edge> &recurrent_edges) {
        int *edges_array = edges_to_array(edges);
        double *edge_weights_array = weights_to_array(edges);

        int *recurrent_edges_array = edges_to_array(recurrent_edges);
        double *recurrent_edge_weights_array = weights_to_array(recurrent_edges);

        int edges_size = edges.size() * 4;
        MPI_Send(&edges_size, 1, MPI_INT, target, EDGES_SIZE_MSG, MPI_COMM_WORLD);
        MPI_Send(edges_array, edges_size, MPI_INT, target, EDGES_ARRAY_MSG, MPI_COMM_WORLD);

        int edge_weights_size = edges.size();
        MPI_Send(&edge_weights_size, 1, MPI_INT, target, EDGES_SIZE_MSG, MPI_COMM_WORLD);
        MPI_Send(edge_weights_array, edge_weights_size, MPI_DOUBLE, target, EDGES_ARRAY_MSG, MPI_COMM_WORLD);

        int recurrent_edges_size = recurrent_edges.size() * 4;
        MPI_Send(&recurrent_edges_size, 1, MPI_INT, target, EDGES_SIZE_MSG, MPI_COMM_WORLD);
        MPI_Send(recurrent_edges_array, recurrent_edges_size, MPI_INT, target, EDGES_ARRAY_MSG, MPI_COMM_WORLD);

        int recurrent_edge_weights_size = recurrent_edges.size();
        MPI_Send(&recurrent_edge_weights_size, 1, MPI_INT, target, EDGES_SIZE_MSG, MPI_COMM_WORLD);
        MPI_Send(recurrent_edge_weights_array, recurrent_edge_weights_size, MPI_DOUBLE, target, EDGES_ARRAY_MSG, MPI_COMM_WORLD);

        delete [] edges_array;
        delete [] edge_weights_array;
        delete [] recurrent_edges_array;
        delete [] recurrent_edge_weights_array;
}

void ant_colony_farmer(int maximum_iterations, AntColony &ant_colony) {
    int max_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &max_rank);

    vector<Edge> edges;
    vector<Edge> recurrent_edges;
    
    //send a network to each worker
    for (int i = 1; i < max_rank; i++) {
        ant_colony.get_ant_paths(edges, recurrent_edges);
        send_edges(i, edges, recurrent_edges);
    }

//    cout << "#iteration f_evals new_fitness max_fitness avg_fitness min_fitness" << endl;
    cout << "#" << setw(9) << "iteration" << setw(20) << "new_fitness" << setw(20) << "max_fitness" << setw(20) << "avg_fitness" << setw(20) << "min_fitness" << endl;
    while (ant_colony.get_iteration() < maximum_iterations) {
        //cout << "farmer probing." << endl;
        MPI_Status status;
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        int source = status.MPI_SOURCE;

        double fitness;
        MPI_Recv(&fitness, 1, MPI_DOUBLE, source, FITNESS_MSG, MPI_COMM_WORLD, &status);

        //receive edges from a worker
        receive_edges(source, edges, recurrent_edges);

        /*
        cout << "received edges: " << endl;
        for (int i = 0; i < edges.size(); i++) {
            cout << edges[i] << endl;
        }
        cout << "received recurrent edges: " << endl;
        for (int i = 0; i < recurrent_edges.size(); i++) {
            cout << recurrent_edges[i] << endl;
        }
        */

        ant_colony.add_ant_paths(fitness, edges, recurrent_edges);

        cout << std::fixed << setw(10) << ant_colony.get_iteration() << setprecision(10) << setw(20) << fitness << setw(20) << ant_colony.get_best_fitness() << setw(20) << ant_colony.get_avg_fitness() << setw(20) << ant_colony.get_worst_fitness() << endl;
//        cout << setw(10) << ant_colony.get_iteration() << setw(10) << evaluations_done << setw(10) << fitness << setw(1) << ant_colony.get_best_fitness() << setw(1) ant_colony.get_avg_fitness() << setw(10) << ant_colony.get_worst_fitness() << endl;

        if (ant_colony.get_iteration() > 0 && (ant_colony.get_iteration() % 100) == 0) {
            ant_colony.write_population();
        }

        //send another set of edges to the worker
        edges.clear();
        recurrent_edges.clear();
        ant_colony.get_ant_paths(edges, recurrent_edges);
        send_edges(source, edges, recurrent_edges);
    }
    MPI_Abort(MPI_COMM_WORLD, 0 /*Success*/);
}

void ant_colony_worker(int rank, double (*objective_function)(vector<Edge> &edges, vector<Edge> &recurrent_edges)) {
    vector<Edge> edges;
    vector<Edge> recurrent_edges;

    bool finished = false;
    while (!finished) {
        //request a neural network from the master
        receive_edges(0, edges, recurrent_edges);

        //evaluate its fitness
        double fitness = objective_function(edges, recurrent_edges);

        //cout << "[worker " << rank << "] calculated fitness: " << fitness << endl;

        //report the fitness
        MPI_Send(&fitness, 1, MPI_DOUBLE, 0, FITNESS_MSG, MPI_COMM_WORLD);
        send_edges(0, edges, recurrent_edges);
    }
}

void ant_colony_optimization_mpi(int maximum_iterations, AntColony &ant_colony, double (*objective_function)(vector<Edge> &edges, vector<Edge> &recurrent_edges)) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        ant_colony_farmer(maximum_iterations, ant_colony);
    } else {
        ant_colony_worker(rank, objective_function);
    }
}
