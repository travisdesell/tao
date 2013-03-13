#include <iostream>
#include <iomanip>

#include "cuda.h"
#include "cuda_runtime.h"
#include "assign_device.hxx"

using namespace std;

void assign_device(int rank, int device) {
    if (device >= 0) {
        cudaSetDevice(device);
        cout << "[worker " << setw(5) << rank << "] set device: " << device << endl;
    }
}
