#include <fstream>
using std::ifstream;

#include <iostream>
using std::ostream;
using std::cerr;
using std::endl;

#include <string>
using std::string;

#include <vector>
using std::vector;

#include <boost/tokenizer.hpp>
using boost::tokenizer;
using boost::char_separator;

#include "edge.hxx"

ostream& operator<< (ostream& out, Edge &edge) {
    out << "[sl: " << edge.src_layer << ", sn: " << edge.src_node << ", dl: " << edge.dst_layer << ", dn: " << edge.dst_node << ", w: " << edge.weight << "]";

    return out;
}

ostream& operator<< (ostream& out, const Edge &edge) {
    out << "[sl: " << edge.src_layer << ", sn: " << edge.src_node << ", dl: " << edge.dst_layer << ", dn: " << edge.dst_node << ", w: " << edge.weight << "]";

    return out;
}

