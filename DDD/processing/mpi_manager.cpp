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

void mpi_manager::send_module(const std::string& parameter, int receiversRank) {
    mpi_communicator::mpi_message message;
    message.header_ = "MSG";
    std::string payload = this->serialize_module_(this, parameter);

    if (payload == "ABORT") {
        std::cout << "SENDING of message aborted\n";
        return;
    }

    message.payload_ = payload;
    mpi_communicator::send_message(message, receiversRank);
}

void mpi_manager::recv_module(const std::string& parameter, int sender) {
    mpi_communicator::mpi_message message;
    mpi_communicator::recv_message(sender, message);

    if (message.header_ == "MSG") {
        this->deserialize_module_(this, parameter, message.payload_);
    } else {
        std::cout << "INVALID MESSAGE TYPE\n";
    }
}

void mpi_manager::complete_instructions(const std::string& instructions, int state) {
    this->calculated_state_ = state;

    std::istringstream inputString(instructions);
    std::string line, keyWord, paramFirst, paramSecond;

    std::cout << "COMPLETING\n";

    while (std::getline(inputString, line)) {
        std::istringstream inputLine(line);

        inputLine >> keyWord;

        if (keyWord == "SEND") {
            inputLine >> paramFirst >> paramSecond;
            this->send_module(paramFirst, std::stoi(paramSecond));
        } else if (keyWord == "RECV") {
            inputLine >> paramFirst >> paramSecond;
            this->recv_module(paramFirst, std::stoi(paramSecond));
        } else {
            this->execute_module_(this, line);
        }
    }
}

void mpi_manager::print_my_modules(int myRank) {
    std::cout << myRank << std::endl;
    for (auto pair : this->my_modules_) {
        pair.second->print_all();
    }
}
