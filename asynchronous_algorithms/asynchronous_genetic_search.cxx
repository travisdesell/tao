
vector<int> random_encoding() {
    vector<int> encoding(row_count);

    for (int i = 0; i < row_count; i++) {
        encoding[i] = rand() % 3;
    }

    return encoding;
}

vector<int> mutate(const vector<int> &encoding) {
    vector<int> mutation(encoding);

    int position = rand() % mutation.size();
    switch (mutation[position]) {
        case 0: mutation[position] = 1 + (rand() % 2); break;
        case 1: if (rand() % 2) {
                    mutation[position] = 0;
                } else {
                    mutation[position] = 2;
                }
                break;
        case 2: mutation[position] = (rand() % 2); break;
        default:
                cerr << "Error, unknown grouping: " << mutation[position] << endl;
                break;
    }

    return mutation;
}

double crossover_rate = 0.5;
bool binary_recombination = true;

vector<int> crossover(const vector<int> &parent1, const vector<int> &parent2) {
    vector<int> child(parent1);

    if (binary_recombination) {
        //Guarantee at least something is taken from the second parent
        int position = rand() % child.size();
        child[position] = parent2[position];

        for (int i = 0; i < child.size(); i++) {
            if (drand48() < crossover_rate) {
                child[i] = parent2[i];
            }
        }

    } else {    //exponential recombination
        int position = rand() % child.size();

        for (int i = position; i < child.size(); i++) {
            child[i] = parent2[i];
        }
    }

    return child;
}


