#ifndef TAO_RECOMBINATION_H
#define TAO_RECOMBINATION_H

#include <vector>
#include <stdint.h>
#include <string>

class Recombination {
    public:
        static void random_parameters(const std::vector<double> &min_bound, const std::vector<double> &max_bound, std::vector<double> &dest);

        static void bound_parameters(const std::vector<double> &min_bound, const std::vector<double> &max_bound, std::vector<double> &dest);

        static void check_bounds(const std::vector<double> &min_bound, const std::vector<double> &max_bound) throw (std::string);

        static void binary_recombination(const std::vector<double> &src1, const std::vector<double> &src2, double crossover_rate, std::vector<double> &dest);

        static void exponential_recombination(const std::vector<double> &src1, const std::vector<double> &src2, double crossover_rate, std::vector<double> &dest);
};

#endif
