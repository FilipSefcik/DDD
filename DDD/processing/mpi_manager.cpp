#include "mpi_manager.hpp"
#include "../utils/mpi_communicator.hpp"
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
    }
    this->my_modules_.clear();
}

void mpi_manager::evaluate(std::string moduleName) {
    module* mod = this->my_modules_.at(moduleName);
    if (mod) {
        std::cout << "Density of " << this->calculated_state_ << ": " << mod->get_reliability(1)
                  << std::endl;
    } else {
        std::cout << "Module not found.\n";
    }
}

void mpi_manager::execute_module(std::string moduleName, int modulePosition) {
    module* mod = this->my_modules_.at(moduleName);
    if (mod) {
        mod->set_position(modulePosition);

        std::string const& path = mod->get_path();
        teddy::bss_manager bssManager(mod->get_var_count(), mod->get_var_count() * 100);
        std::optional<teddy::pla_file> plaFile = teddy::pla_file::load_file(path);
        teddy::bss_manager::diagram_t f =
            bssManager.from_pla(*plaFile, teddy::fold_type::Left)[mod->get_function_column()];

        // const double reliability = bss_manager.calculate_probability(
        //     this->calculated_state_, *mod->get_sons_reliability(), f);

        // const double reliability = this->calculated_state_ == 1 ?
        //                             bss_manager.calculate_availability(1,
        //                             *mod->get_sons_reliability(), f) :
        //                             bss_manager.calculate_unavailability(1,
        //                             *mod->get_sons_reliability(), f);
        // std::cout << mod->get_name() << std::endl;
        // for (int i = 0; i < mod->get_sons_reliability()->size(); i++) {
        //     for (int j = 0; j < mod->get_sons_reliability()->at(i).size(); j++) {
        //         std::cout << mod->get_sons_reliability()->at(i).at(j) << " ";
        //     }
        //     std::cout << std::endl;
        // }

        // std::cout << "avail 1 " << bss_manager.calculate_availability(1,
        // *mod->get_sons_reliability(), f) << std::endl;
        // std::cout << "avail 0 " << bss_manager.calculate_availability(0,
        // *mod->get_sons_reliability(), f) << std::endl;
        // std::cout << "unavail 1 " << bss_manager.calculate_unavailability(1,
        // *mod->get_sons_reliability(), f) << std::endl;
        // std::cout << "unavail 0 " << bss_manager.calculate_unavailability(0,
        // *mod->get_sons_reliability(), f) << std::endl;
        // std::cout << "state 1 " << bss_manager.state_frequency(f, 1) << std::endl;
        // std::cout << "state 0 " << bss_manager.state_frequency(f, 0) << std::endl;
        // std::cout << "prob 1 "
        //           << bss_manager.calculate_probability(1, *mod->get_sons_reliability(), f)
        //           << std::endl;
        // std::cout << "prob 0 "
        //           << bss_manager.calculate_probability(0, *mod->get_sons_reliability(), f)
        //           << std::endl;

        bssManager.calculate_probabilities(*mod->get_sons_reliability(), f);
        std::vector<double> result;
        for (int i = 0; i < 2; i++) {
            std::cout << bssManager.get_probability(i) << " ";
            result.push_back(bssManager.get_probability(i));
        }
        std::cout << std::endl;

        for (double prob : result) {
            std::cout << prob << " ";
        }
        std::cout << std::endl;

        // mod->set_reliability(reliability);
    } else {
        std::cout << "Module not found.\n";
    }
}

void mpi_manager::link_modules(std::string parentName, std::string sonName) {
    module* parent = this->my_modules_.at(parentName);
    module* son = this->my_modules_.at(sonName);
    if (parent && son) {
        parent->set_sons_reliability(son->get_position(), son->get_my_reliabilities());
    } else {
        std::cout << "No module found.\n";
    }
}

void mpi_manager::send_module(std::string moduleName, int receiversRank) {
    module* mod = this->my_modules_.at(moduleName);
    if (mod) {
        mpi_communicator::mpi_message message;
        message.header_ = "MSG";
        message.payload_ = std::to_string(mod->get_position());
        for (double rel : *mod->get_my_reliabilities()) {
            message.payload_ += " " + std::to_string(rel);
        }
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
            std::istringstream line(message.payload_);
            int sonPosition;
            line >> sonPosition;
            std::vector<double> sonRels;
            double temp;
            while (line >> temp) {
                sonRels.push_back(temp);
            }
            parent->set_sons_reliability(sonPosition, &sonRels);
        } else {
            std::cout << "INVALID MESSAGE TYPE\n";
        }
    } else {
        std::cout << "No module found\n";
    }
}

void mpi_manager::complete_instructions(std::string instructions, int state) {
    this->calculated_state_ = state;

    auto inputString = std::istringstream(instructions);

    std::string keyWord, paramFirst, paramSecond;

    while (inputString >> keyWord) {
        if (keyWord == "END") {
            this->evaluate(paramFirst);
        } else {
            inputString >> paramFirst >> paramSecond;
            if (keyWord == "EXEC") {
                this->execute_module(paramFirst, std::stoi(paramSecond));
            } else if (keyWord == "LINK") {
                this->link_modules(paramFirst, paramSecond);
            } else if (keyWord == "SEND") {
                this->send_module(paramFirst, std::stoi(paramSecond));
            } else if (keyWord == "RECV") {
                this->recv_module(paramFirst, std::stoi(paramSecond));
            }
        }
    }
}

void mpi_manager::print_my_modules(int myRank) {
    std::cout << myRank << std::endl;
    for (auto pair : this->my_modules_) {
        std::cout << pair.second->get_name() << std::endl;
    }
}
