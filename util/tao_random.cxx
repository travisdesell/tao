#include <iostream>
using std::cout;
using std::endl;

#include <limits>
using std::numeric_limits;

#include "./util/tao_random.hxx"

TaoRandom::TaoRandom(int _seed) : seed(_seed), n_generated(0), generator(_seed) {
}

uint32_t TaoRandom::operator()() {
    n_generated++;
    return generator();
}

uint32_t TaoRandom::operator()(uint32_t limit) {
    uint32_t result = ((double)generator() / (double)numeric_limits<uint32_t>::max()) * limit;

    n_generated++;
//    cout << "n_generated: " << n_generated << ", limit: " << limit << ", result: " << result << endl;

    return result;
}

void TaoRandom::reset() {
    n_generated = 0;
    generator = mt19937(seed);
}

void TaoRandom::discard(uint64_t count) {
    generator.discard(count);
    n_generated += count;
}

uint32_t TaoRandom::get_seed() {
    return seed;
}

uint64_t TaoRandom::get_n_generated() {
    return n_generated;
}
