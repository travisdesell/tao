#include <iostream>
using std::ostream;

#include <sstream>
using std::ostringstream;

#include <string>
using std::string;

#include "edge_new.hxx"


EdgeNew::EdgeNew(uint32_t _src_depth, uint32_t _src_layer, uint32_t _src_node, uint32_t _dst_depth, uint32_t _dst_layer, uint32_t _dst_node) : src_depth(_src_depth), src_layer(_src_layer), src_node(_src_node), dst_depth(_dst_depth), dst_layer(_dst_layer), dst_node(_dst_node), weight(0.0), weight_sum(0.0), weight_carry(0.0) {
}

EdgeNew::EdgeNew(uint32_t _src_depth, uint32_t _src_layer, uint32_t _src_node, uint32_t _dst_depth, uint32_t _dst_layer, uint32_t _dst_node, double _weight) : src_depth(_src_depth), src_layer(_src_layer), src_node(_src_node), dst_depth(_dst_depth), dst_layer(_dst_layer), dst_node(_dst_node), weight(_weight), weight_sum(0.0), weight_carry(0.0) {
}

string EdgeNew::json() {
    ostringstream oss;
    oss << "{";
    oss << " \"src_depth\" : " << src_depth << ",";
    oss << " \"src_layer\" : " << src_layer << ",";
    oss << " \"src_node\" : " << src_node << ",";
    oss << " \"dst_depth\" : " << dst_depth << ",";
    oss << " \"dst_layer\" : " << dst_layer << ",";
    oss << " \"dst_node\" : " << dst_node << ",";
    oss << " \"weight\" : " << weight;
    oss << "}";

    return oss.str();
}

ostream& operator<< (ostream& out, const EdgeNew &edge) {
    out << "[sd: " << edge.src_depth << ", sl: " << edge.src_layer << ", sn: " << edge.src_node << ", dd: " << edge.dst_depth << ", dl: " << edge.dst_layer << ", dn: " << edge.dst_node << ", w: " << edge.weight << "]";

    return out;
}

ostream& operator<< (ostream& out, const EdgeNew *edge) {
    out << "[sd: " << edge->src_depth << ", sl: " << edge->src_layer << ", sn: " << edge->src_node << ", dd: " << edge->dst_depth << ", dl: " << edge->dst_layer << ", dn: " << edge->dst_node << ", w: " << edge->weight << "]";

    return out;
}



bool operator<(const EdgeNew &e1, const EdgeNew &e2) {
    if (e1.src_layer < e2.src_layer) {
        return true;
    } else if (e1.src_layer > e2.src_layer) {
        return false;
    } else {
        if (e1.src_depth < e2.src_depth) {
            return true;
        } else if (e1.src_depth > e2.src_depth) {
            return false;
        } else {
            if (e1.src_node < e2.src_node) {
                return true;
            } else if (e1.src_node > e2.src_node) {
                return false;
            } else {
                if (e1.dst_layer < e2.dst_layer) {
                    return true;
                } else if (e1.dst_layer > e2.dst_layer) {
                    return false;
                } else {
                    if (e1.dst_depth < e2.dst_depth) {
                        return true;
                    } else if (e1.dst_depth > e2.dst_depth) {
                        return false;
                    } else {
                        if (e1.dst_node < e2.dst_node) {
                            return true;
                        } else if (e1.dst_node > e2.dst_node) {
                            return false;
                        } else {
                            return false;   //the two edges are equal
                        }
                    }
                }

            }
        }
    }

}
