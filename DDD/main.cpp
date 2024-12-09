#include "modules/module.hpp"
#include "processing/module_manager.hpp"
#include "processing/mpi_manager.hpp"
#include "utils/divider.hpp"
#include "utils/mpi_communicator.hpp"
// #include <cstddef>
#include <cstddef>
#include <cstdlib>
// #include <iostream>
// #include <ostream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {

    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int processCount;
    MPI_Comm_size(MPI_COMM_WORLD, &processCount);

    mpi_communicator::mpi_message myMessage;

    if (rank == 0) {
        module_manager manager;
        manager.load("../../DDD/load_files/modules/module_map.conf");
        node_divider divider;
        divider.divide_modules(manager.get_modules(), processCount);

        manager.get_instructions(processCount);

        std::vector<mpi_communicator::mpi_message> messages;
        manager.create_messages(processCount, messages);

        mpi_communicator::scatter_messages(&messages, myMessage);

    } else {
        mpi_communicator::scatter_messages(nullptr, myMessage);
    }

    mpi_manager* mpiManager = nullptr;
    std::vector<module*> modules;

    if (myMessage.header_ == "MODULE") {
        size_t delimiterPos = myMessage.payload_.find(myMessage.delimiter_);
        std::string instructions = myMessage.payload_.substr(0, delimiterPos);
        std::string modules_info = myMessage.payload_.substr(delimiterPos + 3);
        mpiManager = new mpi_manager(modules_info);
        mpiManager->complete_instructions(instructions, 0);
    }

    if (mpiManager) {
        delete mpiManager;
    }

    MPI_Finalize();
    return 0;
}