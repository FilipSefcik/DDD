#include "mpi_manager.hpp"
#include "../utils/mpi_communicator.hpp"
#include <cstdio>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

mpi_manager::mpi_manager(std::string moduleData) {
    std::istringstream input(moduleData);
    std::string line;
    while (std::getline(input, line)) {
        module* mod = new module(line);
        this->my_modules_.emplace(mod->get_name(), mod);
    }
}

mpi_manager::~mpi_manager() {
    for (auto& pair : this->my_modules_) {
        delete pair.second;
        pair.second = nullptr;
    }
    this->my_modules_.clear();
}

void mpi_manager::evaluate(std::string moduleName) {
    module* mod = this->my_modules_.at(moduleName);
    if (mod) {
        std::cout << "Density of " << this->calculated_state_ << ": "
                  << mod->get_reliability(this->calculated_state_) << std::endl;
    } else {
        std::cout << "Module not found.\n";
    }
}

//     module* parent = this->my_modules_.at(parentName);
//     module* son = this->my_modules_.at(sonName);
//     if (parent && son) {
//         parent->set_sons_reliability(son->get_position(), son->get_my_reliabilities());
//     } else {
//         std::cout << "No module found.\n";
//     }
// }

void mpi_manager::send_module(std::string moduleName, int receiversRank) {
    module* mod = this->my_modules_.at(moduleName);
    if (mod) {
        mpi_communicator::mpi_message message;
        message.header_ = "MSG";
        message.payload_ = this->serialize_module_(mod);
        mpi_communicator::send_message(message, receiversRank);
    } else {
        std::cout << "No module found\n";
    }
}

void mpi_manager::recv_module(std::string parentName, int sender) {
    module* parent = this->my_modules_.at(parentName);
    if (parent) {
        mpi_communicator::mpi_message message;
        mpi_communicator::recv_message(sender, message);
        if (message.header_ == "MSG") {
            this->deserialize_module_(message.payload_, parent);
        } else {
            std::cout << "INVALID MESSAGE TYPE\n";
        }
    } else {
        std::cout << "No module found\n";
    }
}

void mpi_manager::complete_instructions(std::string instructions, int state) {
    this->calculated_state_ = state;

    std::istringstream inputString(instructions);
    std::string line, keyWord, paramFirst, paramSecond;

    while (std::getline(inputString, line)) {
        std::istringstream inputLine(line);
        inputLine >> keyWord;
        if (keyWord == "END") {
            this->evaluate(paramFirst);
        } else {
            inputLine >> paramFirst >> paramSecond;
            if (keyWord == "SEND") {
                this->send_module(paramFirst, std::stoi(paramSecond));
            } else if (keyWord == "RECV") {
                this->recv_module(paramFirst, std::stoi(paramSecond));
            } else {
                this->execute_module_(this, line);
            }
        }
    }
}

void mpi_manager::print_my_modules(int myRank) {
    std::cout << myRank << std::endl;
    for (auto pair : this->my_modules_) {
        pair.second->print_all();
    }
}
