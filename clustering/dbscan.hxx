#ifndef TAO_CLUSTERING_DBSCAN
#define TAO_CLUSTERING_DBSCAN

#include <vector>
using std::vector;

void region_query(uint32_t current_sample, const vector< vector<bool> > &within_distance_matrix, vector<int32_t> &neighbor_indices);

template<typename T>
void dbscan(double max_distance, uint32_t min_points, const vector< T > &samples, double (*distance)(const T &, const T &), /*out*/ vector<int32_t> &sample_cluster_assignments, uint32_t &n_clusters);

#endif
