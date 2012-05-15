#include <vector>
#include <stdint.h>
#include <string>
#include <sstream>

#include "recombination.hxx"

/**
 *  Functions dealing with bounds.
 */
void
Recombination::bound_parameters(const std::vector<double> &min_bound, const std::vector<double> &max_bound, std::vector<double> &dest) {
    for (uint32_t i = 0; i < min_bound.size(); i++) {
        if (dest[i] < min_bound[i]) dest[i] = min_bound[i];
        if (dest[i] > max_bound[i]) dest[i] = max_bound[i];
    }
}

void
Recombination::check_bounds(const std::vector<double> &min_bound, const std::vector<double> &max_bound) throw (std::string) {
    if (min_bound.size() != max_bound.size()) {
        std::stringstream oss;
        oss << "ERROR [file: " << __FILE__ << ", line: " << __LINE__ << "]: length of min_bound (" << min_bound.size() << ") was not equal to length of max_bound (" << max_bound.size() << ")"; 
        throw oss.str();
    }   

    for (uint32_t i = 0; i < min_bound.size(); i++) {
        if (min_bound >= max_bound) {
            std::stringstream oss;
            oss << "ERROR [file: " << __FILE__ << ", line: " << __LINE__ << "]: min_bound[" << i << "] (" << min_bound[i] << ") was >= max_bound[" << i << "] (" << max_bound[i] << ")"; 
            throw oss.str();
        }
    }
}

/**
 *  Functions dealing with parameter generation
 */

void
Recombination::random_parameters(const std::vector<double> &min_bound, const std::vector<double> &max_bound, std::vector<double> &dest) {
    if (dest.size() != min_bound.size()) dest.resize(min_bound.size());

    for (uint32_t i = 0; i < min_bound.size(); i++) {
        dest[i] = min_bound[i] + (drand48() * (max_bound[i] - min_bound[i]));   //TODO: not use drand48
    }
}

void
Recombination::binary_recombination(const std::vector<double> &src1, const std::vector<double> &src2, double crossover_rate, std::vector<double> &dest) {
    uint32_t selected = (uint32_t)(drand48() * src1.size());

    for (uint32_t i = 0; i < src1.size(); i++) {
        if (i == selected || drand48() < crossover_rate) {
            dest[i] = src2[i];
        } else {
            dest[i] = src1[i];
        }
    }
}

void
Recombination::exponential_recombination(const std::vector<double> &src1, const std::vector<double> &src2, double crossover_rate, std::vector<double> &dest) {
    uint32_t selected = (uint32_t)(drand48() * src1.size());

    uint32_t i;
    for (i = 0; i < src1.size(); i++) {
        if (i == selected || drand48() < crossover_rate) break;
        dest[i] = src1[i];
    }

    for (; i < src1.size(); i++) dest[i] = src2[i];
}
