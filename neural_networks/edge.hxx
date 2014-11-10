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

        Edge(int sl, int dl, int sn, int dn);
        Edge(int sl, int dl, int sn, int dn, double w);

        friend ostream& operator<< (ostream& out, Edge& edge);
        friend ostream& operator<< (ostream& out, const Edge& edge);
};


ostream& operator<< (ostream& out, Edge &edge);

#endif
