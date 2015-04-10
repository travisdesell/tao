#ifndef TAO_RANDOM_H
#define TAO_RANDOM_H

#include <random>
using std::mt19937;

class TaoRandom { 
    private:
        uint32_t seed;
        uint64_t n_generated;
        mt19937 generator;

    public:
        TaoRandom(int _seed);

        uint32_t operator()();
        uint32_t operator()(uint32_t limit);

        void reset();
        void discard(uint64_t count);

        uint32_t get_seed();
        uint64_t get_n_generated();
};



#endif
