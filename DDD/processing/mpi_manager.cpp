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
        this->my_modules.emplace(mod->get_name(), mod);
    }
}

mpi_manager::~mpi_manager() {
    for (auto& pair : this->my_modules) {
        delete pair.second;
    }

    my_modules.clear();
}

void mpi_manager::evaluate(std::string module_name) {
    module* mod = this->my_modules.at(module_name);
    if (mod) {
        std::cout << "Density of " << this->calculated_state << ": " << mod->get_reliability(1)
                  << std::endl;
    } else {
        std::cout << "module not found.\n";
    }
}

void mpi_manager::execute_module(std::string module_name, int module_position) {
    module* mod = this->my_modules.at(module_name);
    if (mod) {
        mod->set_position(module_position);

        std::string const& path = mod->get_path();
        teddy::bss_manager bss_manager(mod->get_var_count(), mod->get_var_count() * 100);
        std::optional<teddy::pla_file> pla_file = teddy::pla_file::load_file(path);
        teddy::bss_manager::diagram_t f =
            bss_manager.from_pla(*pla_file, teddy::fold_type::Left)[mod->get_function_column()];
        // const double reliability = bss_manager.calculate_probability(
        //     this->calculated_state, *mod->get_sons_reliability(), f);

        // const double reliability = this->calculated_state == 1 ?
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
        // *mod->get_sons_reliability(), f) << std::endl; std::cout << "avail 0 " <<
        // bss_manager.calculate_availability(0, *mod->get_sons_reliability(), f) << std::endl;
        // std::cout << "unavail 1 " << bss_manager.calculate_unavailability(1,
        // *mod->get_sons_reliability(), f) << std::endl; std::cout << "unavail 0 " <<
        // bss_manager.calculate_unavailability(0, *mod->get_sons_reliability(), f) << std::endl;
        // std::cout << "state 1 " << bss_manager.state_frequency(f, 1) << std::endl;
        // std::cout << "state 0 " << bss_manager.state_frequency(f, 0) << std::endl;
        // std::cout << "prob 1 "
        //           << bss_manager.calculate_probability(1, *mod->get_sons_reliability(), f)
        //           << std::endl;
        // std::cout << "prob 0 "
        //           << bss_manager.calculate_probability(0, *mod->get_sons_reliability(), f)
        //           << std::endl;

        bss_manager.calculate_probabilities(*mod->get_sons_reliability(), f);
        std::vector<double> result;
        for (int i = 0; i < 2; i++) {
            std::cout << bss_manager.get_probability(i) << " ";
            result.push_back(bss_manager.get_probability(i));
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

void mpi_manager::link_modules(std::string parent_name, std::string son_name) {
    module* parent = this->my_modules.at(parent_name);
    module* son = this->my_modules.at(son_name);
    if (parent && son) {
        parent->set_sons_reliability(son->get_position(), son->get_my_reliabilities());
    } else {
        std::cout << "No module found.\n";
    }
}

void mpi_manager::send_module(std::string module_name, int recievers_rank) {
    module* mod = this->my_modules.at(module_name);
    if (mod) {
        mpi_communicator::mpi_message message;
        message.header_ = "MSG";
        message.payload_ = std::to_string(mod->get_position());
        for (double rel : *mod->get_my_reliabilities()) {
            message.payload_ += " " + std::to_string(rel);
        }
        mpi_communicator::send_message(message, recievers_rank);
    } else {
        std::cout << "No module found\n";
    }
}

void mpi_manager::recv_module(std::string parent_name, int sender) {
    module* parent = this->my_modules.at(parent_name);
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

    this->calculated_state = state;

    auto input_string = std::istringstream(instructions);

    std::string key_word, param_first, param_second;

    while (input_string >> key_word) {
        if (key_word == "END") {
            this->evaluate(param_first);
        } else {
            input_string >> param_first >> param_second;
            if (key_word == "EXEC") {
                this->execute_module(param_first, std::stoi(param_second));
            } else if (key_word == "LINK") {
                this->link_modules(param_first, param_second);
            } else if (key_word == "SEND") {
                this->send_module(param_first, std::stoi(param_second));
            } else if (key_word == "RECV") {
                this->recv_module(param_first, std::stoi(param_second));
            }
        }
    }
}

void mpi_manager::print_my_modules(int my_rank) {
    std::cout << my_rank << std::endl;
    for (auto pair : this->my_modules) {
        std::cout << pair.second->get_name() << std::endl;
    }
}
