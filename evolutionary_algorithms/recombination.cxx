#include <vector>
#include <stdint.h>
#include <string>
#include <sstream>
#include <cstdlib>

#include "recombination.hxx"

#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"

using boost::variate_generator;
using boost::mt19937;
using boost::uniform_real;

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
Recombination::random_parameters(const std::vector<double> &min_bound, const std::vector<double> &max_bound, std::vector<double> &dest, variate_generator< mt19937,uniform_real<> > *rng) {
    if (dest.size() != min_bound.size()) dest.resize(min_bound.size());

    for (uint32_t i = 0; i < min_bound.size(); i++) {
        dest[i] = min_bound[i] + ((*rng)() * (max_bound[i] - min_bound[i]));
    }
}

void
Recombination::binary_recombination(const std::vector<double> &src1, const std::vector<double> &src2, double crossover_rate, std::vector<double> &dest, variate_generator< mt19937,uniform_real<> > *rng) {
    uint32_t selected = (uint32_t)((*rng)() * src1.size());

    for (uint32_t i = 0; i < src1.size(); i++) {
        if (i == selected || (*rng)() < crossover_rate) {
            dest[i] = src2[i];
        } else {
            dest[i] = src1[i];
        }
    }
}

void
Recombination::exponential_recombination(const std::vector<double> &src1, const std::vector<double> &src2, double crossover_rate, std::vector<double> &dest, variate_generator< mt19937,uniform_real<> > *rng) {
    uint32_t selected = (uint32_t)((*rng)() * src1.size());

    uint32_t i;
    for (i = 0; i < src1.size(); i++) {
        if (i == selected || (*rng)() < crossover_rate) break;
        dest[i] = src1[i];
    }

    for (; i < src1.size(); i++) dest[i] = src2[i];
}
