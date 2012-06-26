#include <cstdlib>
#include <iostream>
#include <vector>

#include "individual.hxx"
#include "vector_io.hxx"

using std::cerr;
using std::endl;

using std::string;
using std::vector;


bool
Individual::operator<(const Individual &rhs) {
    return fitness < rhs.fitness;
}

bool operator<(const Individual &lhs, const Individual &rhs) {
    return lhs.fitness < rhs.fitness;
}

Individual& 
Individual::operator=(const Individual& rhs) { 
    /** THIS SHOULD NEVER BE CALLED **/
    cerr << "ERROR (" << __FILE__ << ":" << __LINE__ << "): Cannot use assignment operator on the individual class as it contains constant fields. (c++ is stupid)" << endl;
    exit(1);
    return *this;
}

std::ostream& operator<< (std::ostream& stream, const Individual &individual) {
    stream << individual.position << ", " << individual.fitness << ", " << vector_to_string(individual.parameters) << ", '" << individual.metadata << "'";
    return stream;
}
