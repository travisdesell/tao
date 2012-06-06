#include <vector>
#include <algorithm>
#include <numeric>

#include "statistics.hxx"

using namespace std;

void calculate_fitness_statistics(const vector<double> &fitness, double &best, double &average, double &median, double &worst) {
    vector<double> fitness_copy(fitness);

    sort(fitness_copy.begin(), fitness_copy.end());

    best = fitness_copy[fitness_copy.size() - 1];
    median = fitness_copy[fitness_copy.size() / 2];
    worst = fitness_copy[0];

    average = accumulate(fitness_copy.begin(), fitness_copy.end(), 0) / fitness_copy.size();
}
