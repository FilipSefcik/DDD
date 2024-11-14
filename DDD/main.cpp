#include <iostream>
#include <mpi.h>

int main(int argc, char* argv[]) {
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Print off a message from each process
    std::cout << "Hello from process " << world_rank << " out of " << world_size << " processes"
              << std::endl;

    // Finalize the MPI environment
    MPI_Finalize();

    return 0;
}
