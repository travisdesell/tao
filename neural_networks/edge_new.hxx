#ifndef TAO_EDGE_NEW_H
#define TAO_EDGE_NEW_H

#include <iostream>
using std::ostream;

#include <string>
using std::string;


class EdgeNew {
    public:
        uint32_t src_depth, src_layer, src_node;
        uint32_t dst_depth, dst_layer, dst_node;

        double weight, weight_sum, weight_carry;

        EdgeNew(uint32_t _src_depth, uint32_t _src_layer, uint32_t _src_node, uint32_t _dst_depth, uint32_t _dst_layer, uint32_t _dst_node);
        EdgeNew(uint32_t _src_depth, uint32_t _src_layer, uint32_t _src_node, uint32_t _dst_depth, uint32_t _dst_layer, uint32_t _dst_node, double _weight);


        string json();

        friend ostream& operator<< (ostream& out, const EdgeNew &edge);
        friend ostream& operator<< (ostream& out, const EdgeNew *edge);

        friend bool operator<(const EdgeNew &e1, const EdgeNew &e2);
};


#endif
