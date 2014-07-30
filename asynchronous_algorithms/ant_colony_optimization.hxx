


class AntColonyOptimization {
    protected:
        double pheromone_degradation_rate = 0.95;

        uint32_t input_layer_size;
        uint32_t hidden_layer_size;
        uint32_t hidden_layers;
        uint32_t output_layer_size;

        double *to_input_pheromones;                    //input_layer_size
        double **input_to_hidden_pheromones;            //input_layer_size x hidden_layer_size
        double ***hidden_to_hidden_pheromones;          //hidden_layers x hidden_layer_size x hidden_layer_size
        double **hidden_to_recurrent_pheromones;        //hidden_layers x hidden_layer_size
        double ****recurrent_to_hidden_pheromones;      //hidden_layers x hidden_layer_size x hidden_layer_size
        double **hidden_to_output_pheromones;           //hidden_layer_size x output_layer_size


    public:
        AntColonyOptimization(  const uint32_t input_layer_size,
                                const uint32_t hidden_layer_size,
                                const uint32_t hidden_layers,
                                const uint32_t output_layer_size ) {

            input_to_hidden_pheromones = new double[input_layer_size];
            for (uint32_t i = 0; i < input_layer_size; i++) {
                input_to_hidden_pheromones[i] = 1.0;
            }

            input_to_hidden_pheromones = new double*[input_layer_size];

            for (uint32_t i = 0; i < input_layer_size; i++) {
                input_to_hidden_pheromones = new double[hidden_layer_size];
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
        }

        vector< pair<int, int> >* get_ant_paths(int number_of_ants) {

        }

        void add_ant_paths(vector< pair<int, int> > *ant_paths) {
        }

        void decrease_pheromones() {
            for (uint32_t i = 0; i < input_layer_size; i++) {
                to_input_paramaeters[i] = fmax(1.0, to_input_parameters[i] * pheromone_degradation_rate);
            }

            for (uint32_t i = 0; i < input_layer_size; i++) {
                for (uint32_t j = 0; j < hidden_layer_size; j++) {
                    input_to_hidden_pheromones[i][j] = fmax(1.0, input_to_hidden_pheromones[i][j] * pheromone_degradation_rate);
                }
            }

            for (uint32_t i = 0; i < hidden_layers; i++) {
                for (uint32_t j = 0; j < hidden_layer_size; j++) {
                    for (uint32_t k = 0; k < hidden_layer_size; k++) {
                        hidden_to_hidden_pheromones[i][j][k] = fmax(1.0, hidden_to_hidden_pheromones[i][j][k] * pheromone_degradation_rate);
                        recurrent_to_hidden_pheromones[i][j][k] = fmax(1.0, recurrent_to_hidden_pheromones[i][j][k] * pheromone_degradation_rate);
                    }
                }
            }

            for (uint32_t i = 0; i < hidden_layers; i++) {
                for (uint32_t j = 0; j < hidden_layer_size; j++) {
                    hidden_to_recurrent_pheromones[i][j] = fmax(1.0, hidden_to_recurrent_pheromones[i][j] * pheromone_degradation_rate);
                }
            }

            for (uint32_t i = 0; i < hidden_layer_size; i++) {
                for (uint32_t j = 0; j < output_layer_size; j++) {
                    hidden_to_output_pheromones[i][j] = fmax(1.0, hidden_to_pheromones[i][j] * pheromone_degradation_rate);
                }
            }
        }
}
