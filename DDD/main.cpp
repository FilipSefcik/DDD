// #include "modules/module.hpp"
// #include "processing/module_manager.hpp"
// #include "utils/divider.hpp"
// #include "utils/mpi_communicator.hpp"
// #include <cstddef>
// #include <iostream>
// #include <ostream>
// #include <vector>

// int main(int argc, char* argv[]) {

//     module_manager manager;
//     manager.load("../../DDD/load_files/modules/module_map.conf");
//     // manager.print_modules();
//     std::vector<module*>* nodes = new std::vector<module*>[3];
//     std::vector<int>* moduleCount = new std::vector<int>;
//     moduleCount->resize(3);

//     node_divider divider;
//     divider.divide_modules(manager.get_modules(), nodes, moduleCount);

//     for (int i = 0; i < 3; i++) {
//         std::vector<module*> nodes_modules = nodes[i];
//         std::cout << "Node " << i << std::endl;

//         for (module* mod : nodes_modules) {
//             mod->print_sons();
//         }
//         std::cout << std::endl;
//     }

//     manager.get_instructions(3);
//     // manager.print_assigned_processes();
//     // manager.print_separate_instructions();
//     std::string instr = manager.get_instructions_for_process(2);
//     std::cout << instr << std::endl;

//     MPI_Init(&argc, &argv);

//     int rank;
//     MPI_Comm_rank(MPI_COMM_WORLD, &rank);

//     int processCount;
//     MPI_Comm_size(MPI_COMM_WORLD, &processCount);

//     if (rank == 0) {
//         module_manager manager;
//         manager.load("../../DDD/load_files/modules/module_map.conf");
//         // manager.print_modules();
//         std::vector<module*>* nodes = new std::vector<module*>[processCount];
//         std::vector<int>* moduleCount = new std::vector<int>;
//         moduleCount->resize(processCount);

//         node_divider divider;
//         divider.divide_modules(manager.get_modules(), nodes, moduleCount);

//         int assignedCount;
//         mpi_communicator::scatter_ints(moduleCount->data(), &assignedCount);

//     } else {
//         int assginedCount;
//         mpi_communicator::scatter_ints(nullptr, &assginedCount);
//     }

//     MPI_Finalize();
//     return 0;
// }

#include "utils/mpi_communicator.hpp"
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        // Root process sends messages
        std::vector<mpi_communicator::mpi_message> messages = {{"Header1", "Payload1"},
                                                               {"Header2", "Payload2"},
                                                               {"Header3", "Payload3"},
                                                               {"Header4", "Payload4"},
                                                               {"UNUSED", "SORRY PAL :("}};
        mpi_communicator::mpi_message receivedMessage;
        mpi_communicator::scatter_messages(messages, receivedMessage);
    } else {
        // Other processes receive messages
        mpi_communicator::mpi_message receivedMessage;
        mpi_communicator::scatter_messages({}, receivedMessage);

        std::cout << "Rank " << rank << " received:\n";
        std::cout << "Header: " << receivedMessage.header_ << "\n";
        std::cout << "Payload: " << receivedMessage.payload_ << "\n";
    }

    MPI_Finalize();
    return 0;
}
