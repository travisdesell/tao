#include <vector>
#include <algorithm>
#include <numeric>

#include "stdint.h"

#include "individual.hxx"
#include "statistics.hxx"

using namespace std;

void calculate_fitness_statistics(const vector<Individual> &individuals, double &best, double &average, double &median, double &worst) {
    vector<Individual> individuals_copy(individuals);

    sort(individuals_copy.begin(), individuals_copy.end());

    best = individuals_copy[individuals_copy.size() - 1].fitness;
    median = individuals_copy[individuals_copy.size() / 2].fitness;
    worst = individuals_copy[0].fitness;

    average = 0;
    for (uint32_t i = 0; i < individuals_copy.size(); i++) {
        average += individuals_copy[i].fitness;
    }
    average = average / individuals_copy.size();
}

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
