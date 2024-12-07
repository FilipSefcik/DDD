#include "modules/module.hpp"
#include "processing/module_manager.hpp"
#include "utils/divider.hpp"
#include "utils/mpi_communicator.hpp"
// #include <cstddef>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <vector>

int main(int argc, char* argv[]) {

    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int processCount;
    MPI_Comm_size(MPI_COMM_WORLD, &processCount);

    if (rank == 0) {
        module_manager manager;
        manager.load("../../DDD/load_files/modules/module_map.conf");
        node_divider divider;
        divider.divide_modules(manager.get_modules(), processCount);

        std::vector<mpi_communicator::mpi_message> messages;
        manager.create_messages(processCount, messages);
        std::vector<module*> modules;

        int i = 0;
        for (mpi_communicator::mpi_message message : messages) {
            std::cout << i << std::endl;
            std::cout << message.header_ << std::endl << message.payload_ << std::endl;
            i++;
            if (message.header_ == "MODULE") {
                modules.push_back(new module(message.payload_));
            }
        }

        for (size_t i = 0; i < modules.size(); i++) {
            modules.at(i)->print_all();
            delete modules.at(i);
        }

    } else {
    }

    MPI_Finalize();
    return 0;
}

// #include "utils/mpi_communicator.hpp"
// #include <iostream>
// #include <vector>

// int main(int argc, char** argv) {
//     MPI_Init(&argc, &argv);

//     int rank;
//     MPI_Comm_rank(MPI_COMM_WORLD, &rank);

//     if (rank == 0) {
//         // Root process sends messages
//         std::vector<mpi_communicator::mpi_message> messages = {{"Header1", "Payload1"},
//                                                                {"Header2", "Payload2"},
//                                                                {"Header3", "Payload3"},
//                                                                {"Header4", "Payload4"},
//                                                                {"UNUSED", "SORRY PAL :("}};
//         mpi_communicator::mpi_message receivedMessage;
//         mpi_communicator::scatter_messages(messages, receivedMessage);
//     } else {
//         // Other processes receive messages
//         mpi_communicator::mpi_message receivedMessage;
//         mpi_communicator::scatter_messages({}, receivedMessage);

//         std::cout << "Rank " << rank << " received:\n";
//         std::cout << "Header: " << receivedMessage.header_ << "\n";
//         std::cout << "Payload: " << receivedMessage.payload_ << "\n";
//     }

//     MPI_Finalize();
//     return 0;
// }
