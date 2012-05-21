#ifndef TAO_RECOMBINATION_H
#define TAO_RECOMBINATION_H

#include <vector>
#include <stdint.h>
#include <string>

#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"

using boost::variate_generator;
using boost::mt19937;
using boost::uniform_real;


class Recombination {
    public:

        static void bound_parameters(const std::vector<double> &min_bound, const std::vector<double> &max_bound, std::vector<double> &dest);

        static void check_bounds(const std::vector<double> &min_bound, const std::vector<double> &max_bound) throw (std::string);

        static void random_parameters(const std::vector<double> &min_bound, const std::vector<double> &max_bound, std::vector<double> &dest, variate_generator< mt19937,uniform_real<> > *rng);

        static void binary_recombination(const std::vector<double> &src1, const std::vector<double> &src2, double crossover_rate, std::vector<double> &dest, variate_generator< mt19937,uniform_real<> > *rng);

        static void exponential_recombination(const std::vector<double> &src1, const std::vector<double> &src2, double crossover_rate, std::vector<double> &dest, variate_generator< mt19937,uniform_real<> > *rng);
};

#endif
