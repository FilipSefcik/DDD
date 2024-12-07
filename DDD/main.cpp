#include "modules/module.hpp"
#include "processing/module_manager.hpp"
#include "utils/divider.hpp"
#include "utils/mpi_communicator.hpp"
// #include <cstddef>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <string>
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

        manager.get_instructions(processCount);

        std::vector<mpi_communicator::mpi_message> messages;
        manager.create_messages(processCount, messages);

        mpi_communicator::mpi_message myMessage;
        mpi_communicator::scatter_messages(&messages, myMessage);

        std::vector<module*> modules;
        std::cout << "RANK " << rank << std::endl;
        std::cout << myMessage.header_ << std::endl << myMessage.payload_ << std::endl;

        size_t delimiterPos = myMessage.payload_.find(myMessage.delimiter_);
        std::string instructions = myMessage.payload_.substr(0, delimiterPos);
        std::string modules_info = myMessage.payload_.substr(delimiterPos + 3);

        if (myMessage.header_ == "MODULE") {
            std::istringstream payload(modules_info);
            std::string line;
            while (std::getline(payload, line)) {
                modules.push_back(new module(line));
            }
        }

        for (size_t i = 0; i < modules.size(); i++) {
            modules.at(i)->print_all();
            delete modules.at(i);
        }
    } else {
        mpi_communicator::mpi_message myMessage;
        mpi_communicator::scatter_messages(nullptr, myMessage);

        std::vector<module*> modules;
        std::cout << "RANK " << rank << std::endl;
        std::cout << myMessage.header_ << std::endl << myMessage.payload_ << std::endl;

        size_t delimiterPos = myMessage.payload_.find(myMessage.delimiter_);
        std::string instructions = myMessage.payload_.substr(0, delimiterPos);
        std::string modules_info = myMessage.payload_.substr(delimiterPos + 3);

        if (myMessage.header_ == "MODULE") {
            std::istringstream payload(modules_info);
            std::string line;
            while (std::getline(payload, line)) {
                modules.push_back(new module(line));
            }
        }

        for (size_t i = 0; i < modules.size(); i++) {
            modules.at(i)->print_all();
            delete modules.at(i);
        }
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
