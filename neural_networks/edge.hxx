#ifndef TAO_EDGE_H
#define TAO_EDGE_H

#include <iostream>
using std::ostream;

class Edge {
    public:
        int src_layer;
        int dst_layer;
        int src_node;
        int dst_node;

        double weight;

        Edge(int sl, int dl, int sn, int dn) : src_layer(sl), dst_layer(dl), src_node(sn), dst_node(dn), weight(0.0) {}

        friend ostream& operator<< (ostream& out, Edge& edge);
};


ostream& operator<< (ostream& out, Edge &edge);

#endif
