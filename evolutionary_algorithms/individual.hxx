#ifndef TAO_EA_INDIVIDUAL_H
#define TAO_EA_INDIVIDUAL_H

#include <iostream>
#include <vector>

#include "vector_io.hxx"

using std::cerr;
using std::endl;

using std::string;
using std::vector;

class Individual {
    public:
        const int position;
        const double fitness;
        const vector<double> parameters;
        const string metadata;

        Individual(int position, double fitness, const vector<double> &parameters, const string &metadata) : position(position), fitness(fitness), parameters(parameters), metadata(metadata) {}
        virtual ~Individual() {}

        bool operator<(const Individual &rhs);
        friend bool operator<(const Individual &lhs, const Individual &rhs);

        Individual& operator=(const Individual& rhs);

        friend std::ostream& operator<< (std::ostream& stream, const Individual &individual);
};

#endif
