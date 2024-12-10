// #include "modules/module.hpp"
// #include "processing/module_manager.hpp"
// #include "processing/mpi_manager.hpp"
#include "processing/process.hpp"
// #include "utils/divider.hpp"
// #include "utils/mpi_communicator.hpp"
// #include <cstddef>
// #include <cstddef>
#include <cstdlib>
#include <iostream>
// #include <ostream>
#include <string>
// #include <vector>

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int processCount;
    MPI_Comm_size(MPI_COMM_WORLD, &processCount);

    char processorName[MPI_MAX_PROCESSOR_NAME];
    int nameLength;
    MPI_Get_processor_name(processorName, &nameLength);

    // std::cout << "Rank " << rank << " runs on " << processorName << std::endl;

    process* process = nullptr;

    if (rank == 0) {
        auto* mainProcess = new main_process(rank, processCount);
        mainProcess->set_conf_path("/home/sefcik1/mpi_cloud/modules/module_map.conf");
        mainProcess->set_divider(1);
        process = mainProcess;
    } else {
        process = new slave_process(rank);
    }

    process->process_information();
    process->process_instructions(1);

    delete process; // Virtual destructor ensures proper cleanup
    //  std::cout << rank << " PROCESS DELETE OK\n";

    MPI_Finalize();
    return 0;
}
