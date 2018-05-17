#include <algorithm>
using std::set_union;

#include <fstream>
using std::ifstream;

#include <iomanip>
using std::setw;

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include <cmath>

#include <string>
using std::string;
using std::stoi;

#include <vector>
using std::vector;

#include "clustering/dbscan.hxx"

void region_query(uint32_t current_sample, const vector< vector<bool> > &within_distance_matrix, vector<int32_t> &neighbor_indices) {
    //perform region query
    for (uint32_t potential_neighbor = 0; potential_neighbor < within_distance_matrix[current_sample].size(); potential_neighbor++) {
        if (within_distance_matrix[current_sample][potential_neighbor] == true) {
            neighbor_indices.push_back(potential_neighbor);
        }
    }
}


template<typename T>
void dbscan(double max_distance, uint32_t min_points, const vector< T > &samples, double (*distance)(const T &, const T &), /*out*/ vector<int32_t> &sample_cluster_assignments, uint32_t &n_clusters) {
    cout << "in dbscan, samples.size(): " << samples.size() << endl;
    vector< vector<double> > distance_matrix(samples.size(), vector<double>(samples.size()));
    cout << "allocated distance matrix" << endl;
    vector< vector<bool> > within_distance_matrix(samples.size(), vector<bool>(samples.size()));
    cout << "allocated within distance matrix" << endl;

    const int32_t UNASSIGNED = -2;
    const int32_t NOISE = -1;
    int32_t current_cluster = 0;
    sample_cluster_assignments = vector<int32_t>(samples.size(), UNASSIGNED);
    vector<uint32_t> neighbor_counts(samples.size(), 0);
    vector<bool> sample_visited(samples.size(), false);

    vector<uint64_t> histogram(40, 0);
    uint64_t count = 0;
    for (uint32_t i = 0; i < samples.size(); i++) {
        distance_matrix[i][i] = 0.0;
        within_distance_matrix[i][i] = true;

        for (uint32_t j = i + 1; j < samples.size(); j++) {
            distance_matrix[i][j] = distance(samples[i], samples[j]);
            distance_matrix[j][i] = distance_matrix[i][j];

            histogram[(uint32_t)distance_matrix[i][j]]++;
            count++;

            if (distance_matrix[i][j] <= max_distance) {
                within_distance_matrix[i][j] = true;
                within_distance_matrix[j][i] = true;
                neighbor_counts[i]++;
                neighbor_counts[j]++;
            } else {
                within_distance_matrix[i][j] = false;
                within_distance_matrix[j][i] = false;
            }

            if (count % 1000000 == 0) {
                cout << "i: " << i << ", count: " << count << ", distance counts:";
                for (uint32_t k = 0; k < histogram.size(); k++) {
                    cout << " " << histogram[k];
                }
                cout << endl;
            }
        }

        if (neighbor_counts[i] < min_points) {
            sample_cluster_assignments[i] = NOISE;
            sample_visited[i] = true;
        }
    }


    for (uint32_t current_sample = 0; current_sample < samples.size(); current_sample++) {
        if (sample_visited[current_sample]) continue;

        sample_visited[current_sample] = true;

        cout << "current_sample: " << current_sample << ", neighbor_counts: " << neighbor_counts[current_sample] << endl;

        if (neighbor_counts[current_sample] >= min_points) {
            vector<int32_t> neighbor_indices;
            region_query(current_sample, within_distance_matrix, neighbor_indices);
            //cout << "\tneighbor_indices.size(): " << neighbor_indices.size() << endl;

            //expland cluster
            current_cluster++;
            //cout << "\tcurrent cluster: " << current_cluster << endl;
            sample_cluster_assignments[current_sample] = current_cluster;

            for (uint32_t i = 0; i < neighbor_indices.size(); i++) {
                uint32_t neighbor_index = neighbor_indices[i];
                //cout << "\ti: " << i << ", neighbor_index: " << neighbor_index << endl;

                if (!sample_visited[neighbor_index]) {
                    //perform region query
                    sample_visited[neighbor_index] = true;

                    vector<int32_t> new_neighbor_indices;
                    region_query(neighbor_index, within_distance_matrix, new_neighbor_indices);
                    //cout << "\tnew_neighbor_indices.size(): " << new_neighbor_indices.size() << endl;

                    if (new_neighbor_indices.size() >= min_points) {
                        vector<int32_t> merged_neighbors;
                        /*
                        cout << "\tmerging neighbors" << endl;

                        cout << "\t\tneighbor_indices:";
                        for (uint32_t j = 0; j < neighbor_indices.size(); j++) cout << " " << neighbor_indices[j];
                        cout << endl;

                        cout << "\t\tnew_neighbor_indices:";
                        for (uint32_t j = 0; j < new_neighbor_indices.size(); j++) cout << " " << new_neighbor_indices[j];
                        cout << endl;
                        */


                        //need to have enough space available
                        merged_neighbors.resize(neighbor_indices.size() + new_neighbor_indices.size());

                        vector<int>::iterator it = set_union(neighbor_indices.begin(), neighbor_indices.end(), new_neighbor_indices.begin(), new_neighbor_indices.end(), merged_neighbors.begin());
                        merged_neighbors.resize(it - merged_neighbors.begin());
                        neighbor_indices.assign(merged_neighbors.begin(), merged_neighbors.end());

                        //this is a bad hack, but it has to be done in the case of adding indices that are inserted before i
                        i = 0;
                    }

                    /*
                    cout << "\tafter merge:";
                    for (uint32_t j = 0; j < neighbor_indices.size(); j++) cout << " " << neighbor_indices[j];
                    cout << endl;
                    cout << endl;
                    */

                }

                if (sample_cluster_assignments[neighbor_index] == UNASSIGNED) {
                    sample_cluster_assignments[neighbor_index] = current_cluster;
                }
            }
        }
    }

    n_clusters = current_cluster;
}



//NEED TO DEFINE DBSCAN FOR DIFFERENT SAMPLE TYPES (thanks C++)
template void dbscan(double max_distance, uint32_t min_points, const vector< string > &samples, double (*distance)(const string &, const string &), /*out*/ vector<int32_t> &sample_cluster_assignments, uint32_t &n_clusters);

template void dbscan(double max_distance, uint32_t min_points, const vector< vector<double> > &samples, double (*distance)(const vector<double> &, const vector<double> &), /*out*/ vector<int32_t> &sample_cluster_assignments, uint32_t &n_clusters);


