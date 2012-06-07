#include <vector>
#include <algorithm>
#include <numeric>

#include "stdint.h"

#include "statistics.hxx"

using namespace std;

void calculate_fitness_statistics(const vector<double> &fitness, double &best, double &average, double &median, double &worst) {
    vector<double> fitness_copy(fitness);

    sort(fitness_copy.begin(), fitness_copy.end());

    best = fitness_copy[fitness_copy.size() - 1];
    median = fitness_copy[fitness_copy.size() / 2];
    worst = fitness_copy[0];

    average = 0;
    for (uint32_t i = 0; i < fitness_copy.size(); i++) {
        average += fitness_copy[i];
    }
    average = average / fitness_copy.size();
}
