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
    out << "[sl: " << edge.src_layer << ", dl: " << edge.dst_layer << ", sn: " << edge.src_node << ", dn: " << edge.dst_node << "]";

    return out;
}

