// #include "modules/module.hpp"
// #include "modules/module_info.hpp"
#include "processing/module_manager.hpp"
#include "utils/divider.hpp"
#include "utils/mpi_communicator.hpp"
// #include <cstddef>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <vector>

int main(int argc, char* argv[]) {

    // module_manager manager;
    // manager.load("../../DDD/load_files/modules/module_map.conf");
    // // manager.print_modules();
    // std::vector<module_info*>* nodes = new std::vector<module_info*>[3];
    // std::vector<int>* moduleCount = new std::vector<int>;
    // moduleCount->resize(3);

    // node_divider divider;
    // divider.divide_modules(manager.get_modules(), nodes, moduleCount);

    // manager.get_instructions(3);
    // manager.print_assigned_processes();
    // manager.print_separate_instructions();

    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int processCount;
    MPI_Comm_size(MPI_COMM_WORLD, &processCount);

    if (rank == 0) {
        module_manager manager;
        manager.load("../../DDD/load_files/modules/module_map.conf");
        // manager.print_modules();

        node_divider divider;
        divider.divide_modules(manager.get_modules(), processCount);

        // for (module_info* mod : *nodes) {
        //     std::cout << mod->to_string() << std::endl;
        // }

        std::vector<mpi_communicator::mpi_message> messages;
        manager.create_messages(processCount, messages);

        int i = 0;
        for (mpi_communicator::mpi_message message : messages) {
            std::cout << i << std::endl;
            std::cout << message.header_ << std::endl << message.payload_ << std::endl;
            i++;
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
