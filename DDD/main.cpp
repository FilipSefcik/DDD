#include "modules/module.hpp"
#include "processing/module_manager.hpp"
#include "utils/divider.hpp"
#include "utils/mpi_communicator.hpp"
#include <iostream>
#include <ostream>
#include <vector>

int main(int argc, char* argv[]) {

    // module_manager manager;
    // manager.load("../../DDD/load_files/modules/module_map.conf");
    // // manager.print_modules();
    // std::vector<module*>* nodes = new std::vector<module*>[3];
    // std::vector<int>* moduleCount = new std::vector<int>;
    // moduleCount->resize(3);

    // node_divider divider;
    // divider.divide_modules(manager.get_modules(), nodes, moduleCount);

    // for (int i = 0; i < 3; i++) {
    //     std::vector<module*> nodes_modules = nodes[i];
    //     std::cout << "Node " << i << std::endl;

    //     for (module* mod : nodes_modules) {
    //         mod->print_sons();
    //     }
    //     std::cout << std::endl;
    // }

    // manager.get_instructions(3);
    // // manager.print_assigned_processes();
    // // manager.print_separate_instructions();
    // std::string instr = manager.get_instructions_for_process(2);
    // std::cout << instr << std::endl;

    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        // Sender
        mpi_communicator::mpi_message message;
        message.header_ = "HeaderData";
        message.payload_ = "This is the payload of the message.";
        mpi_communicator::send_message(message, 1);
    } else if (rank == 1) {
        // Receiver
        mpi_communicator::mpi_message received_message;
        mpi_communicator::recv_message(0, received_message);

        std::cout << "Received:\n";
        std::cout << "Header: " << received_message.header_ << "\n";
        std::cout << "Payload: " << received_message.payload_ << "\n";
    }

    MPI_Finalize();
    return 0;
}