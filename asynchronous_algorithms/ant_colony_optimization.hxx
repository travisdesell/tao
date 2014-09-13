#ifndef TAO_ANT_COLONY_OPTIMIZATION_H
#define TAO_ANT_COLONY_OPTIMIZATION_H

#include <cmath>
#include <vector>
#include <utility>

#include "stdint.h"

using std::pair;
using std::vector;

class AntColonyOptimization {
    protected:
        const static double PHEROMONE_DEGRADATION_RATE;

        uint32_t input_layer_size;
        uint32_t hidden_layer_size;
        uint32_t hidden_layers;
        uint32_t output_layer_size;

        double *to_input_pheromones;                    //input_layer_size
        double **input_to_hidden_pheromones;            //input_layer_size x hidden_layer_size
        double ***hidden_to_hidden_pheromones;          //hidden_layers x hidden_layer_size x hidden_layer_size
        double **hidden_to_recurrent_pheromones;        //hidden_layers x hidden_layer_size
        double ***recurrent_to_hidden_pheromones;      //hidden_layers x hidden_layer_size x hidden_layer_size
        double **hidden_to_output_pheromones;           //hidden_layer_size x output_layer_size


    public:
        AntColonyOptimization(  const uint32_t input_layer_size,
                                const uint32_t hidden_layer_size,
                                const uint32_t hidden_layers,
                                const uint32_t output_layer_size ) {

            to_input_pheromones = new double[input_layer_size];
            for (uint32_t i = 0; i < input_layer_size; i++) {
                to_input_pheromones[i] = 1.0;
            }

            input_to_hidden_pheromones = new double*[input_layer_size];

            for (uint32_t i = 0; i < input_layer_size; i++) {
                input_to_hidden_pheromones[i] = new double[hidden_layer_size];
                for (uint32_t j = 0; j < hidden_layer_size; j++) {
                    input_to_hidden_pheromones[i][j] = 1.0;
                }
            }

            hidden_to_hidden_pheromones = new double**[hidden_layers];
            recurrent_to_hidden_pheromones = new double**[hidden_layers];
            for (uint32_t i = 0; i < hidden_layers; i++) {
                hidden_to_hidden_pheromones[i] = new double*[hidden_layer_size];
                recurrent_to_hidden_pheromones[i] = new double*[hidden_layer_size];
                for (uint32_t j = 0; j < hidden_layer_size; j++) {
                    hidden_to_hidden_pheromones[i][j] = new double[hidden_layer_size];
                    recurrent_to_hidden_pheromones[i][j] = new double[hidden_layer_size];
                    for (uint32_t k = 0; k < hidden_layer_size; k++) {
                        hidden_to_hidden_pheromones[i][j][k] = 1.0;
                        recurrent_to_hidden_pheromones[i][j][k] = 1.0;
                    }
                }
            }

            hidden_to_recurrent_pheromones = new double*[hidden_layers];
            for (uint32_t i = 0; i < hidden_layers; i++) {
                hidden_to_recurrent_pheromones[i] = new double[hidden_layer_size];
                for (uint32_t j = 0; j < hidden_layer_size; j++) {
                    hidden_to_recurrent_pheromones[i][j] = 1.0;
                }
            }

            hidden_to_output_pheromones = new double*[hidden_layer_size];
            for (uint32_t i = 0; i < hidden_layer_size; i++) {
                hidden_to_output_pheromones[i] = new double[output_layer_size];
                for (uint32_t j = 0; j < output_layer_size; j++) {
                    hidden_to_output_pheromones[i][j] = 1.0;
                }
            }
        }

        ~AntColonyOptimization() {
            delete[] to_input_pheromones;

            for (uint32_t i = 0; i < input_layer_size; i++) {
                delete[] input_to_hidden_pheromones[i];
            }
            delete[] input_to_hidden_pheromones;

            for (uint32_t i = 0; i < hidden_layers; i++) {
                for (uint32_t j = 0; j < hidden_layer_size; j++) {
                    delete[] hidden_to_hidden_pheromones[i][j];
                    delete[] recurrent_to_hidden_pheromones[i][j];
                }
                delete[] hidden_to_hidden_pheromones[i];
                delete[] recurrent_to_hidden_pheromones[i];
            }
            delete[] hidden_to_hidden_pheromones;
            delete[] recurrent_to_hidden_pheromones;

            for (uint32_t i = 0; i < hidden_layers; i++) {
                delete[] hidden_to_recurrent_pheromones[i];
            }
            delete[] hidden_to_recurrent_pheromones;

            for (uint32_t i = 0; i < hidden_layer_size; i++) {
                delete[] hidden_to_output_pheromones[i];
            }
            delete[] hidden_to_output_pheromones;
        }

        vector<Edge>* get_ant_paths(int number_of_ants) {
            vector<Edge> *paths = new vector<Edge>();
            vector<int> positions(number_of_ants, 0);
            double sum = 0.0;

            //calculate sum of pheromones to the input layer
            for (int i = 0; i < input_layer_size; i++) {
                sum += to_input_pheromones[i];
            }

            //path to input node
            for (int i = 0; i < number_of_ants; i++) {
                double rand_num = mt_random() * sum;

                for (int j = 0; j < input_layer_size; j++) {
                    if (rand_num < to_input_pheromones[j]) {
                        paths->push_back( Edge(0, j, false) );
                        positions[i] = j;
                        break;
                    } else {
                        rand_num -= to_input_pheromones[j];
                    }
                }

            }

            //path to first hidden node
            for (int i = 0; i < number_of_ants; i++) {
                sum = 0.0;
                for (int j = 0; j < hidden_layer_size; j++) {
                    sum += input_to_hidden_pheromones[positions[i]][j];
                }

                double rand_num = mt_random() * sum;

                for (int j = 0; j < hidden_layer_size; j++) {
                    if (rand_num < input_to_hidden_pheromones[positions[i]][j]) {
                        paths->push_back( Edge(positions[i], j, false) );
                        positions[i] = j;
                        break;
                    } else {
                        rand_num -= input_to_hidden_pheromones[positions[i]][j];
                    }
                }
            }

            //paths from hidden to hidden or hidden to recurrent nodes
            for (int i = 0; i < number_of_ants; i++) {
                for (int k = 0; k < hidden_layers; k++) {
                    double rand_num = mt_random() * sum;

                }
            }


            return NULL;

        }

        void add_ant_paths(vector< pair<int, int> > *ant_paths) {
        }

        void decrease_pheromones() {
            for (uint32_t i = 0; i < input_layer_size; i++) {
                to_input_pheromones[i] = fmax(1.0, to_input_pheromones[i] * PHEROMONE_DEGRADATION_RATE);
            }

            for (uint32_t i = 0; i < input_layer_size; i++) {
                for (uint32_t j = 0; j < hidden_layer_size; j++) {
                    input_to_hidden_pheromones[i][j] = fmax(1.0, input_to_hidden_pheromones[i][j] * PHEROMONE_DEGRADATION_RATE);
                }
            }

            for (uint32_t i = 0; i < hidden_layers; i++) {
                for (uint32_t j = 0; j < hidden_layer_size; j++) {
                    for (uint32_t k = 0; k < hidden_layer_size; k++) {
                        hidden_to_hidden_pheromones[i][j][k] = fmax(1.0, hidden_to_hidden_pheromones[i][j][k] * PHEROMONE_DEGRADATION_RATE);
                        recurrent_to_hidden_pheromones[i][j][k] = fmax(1.0, recurrent_to_hidden_pheromones[i][j][k] * PHEROMONE_DEGRADATION_RATE);
                    }
                }
            }

            for (uint32_t i = 0; i < hidden_layers; i++) {
                for (uint32_t j = 0; j < hidden_layer_size; j++) {
                    hidden_to_recurrent_pheromones[i][j] = fmax(1.0, hidden_to_recurrent_pheromones[i][j] * PHEROMONE_DEGRADATION_RATE);
                }
            }

            for (uint32_t i = 0; i < hidden_layer_size; i++) {
                for (uint32_t j = 0; j < output_layer_size; j++) {
                    hidden_to_output_pheromones[i][j] = fmax(1.0, hidden_to_output_pheromones[i][j] * PHEROMONE_DEGRADATION_RATE);
                }
            }
        }
};

#endif
