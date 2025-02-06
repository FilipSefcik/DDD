#include "mpi_manager.hpp"
#include "../utils/mpi_communicator.hpp"
#include "libteddy/inc/core.hpp"
#include <iostream>
#include <libteddy/impl/diagram_manager.hpp>
#include <libteddy/impl/types.hpp>
#include <libteddy/inc/io.hpp>
#include <libteddy/inc/reliability.hpp>
#include <libtsl/pla-description.hpp>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

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

void mpi_manager::execute_module(std::string moduleName, int modulePosition) {
    module* mod = this->my_modules_.at(moduleName);
    if (mod) {
        mod->set_position(modulePosition);

        std::string const& path = mod->get_path();

        // teddy::ifmss_manager<2> ifmssManager(mod->get_var_count(), mod->get_var_count() * 100,
        //                                      *mod->get_son_rel_count());
        // teddy::imss_manager ifmssManager(mod->get_var_count(), mod->get_var_count() * 100,
        //                                  *mod->get_son_rel_count());

        teddy::bss_manager manager(mod->get_var_count(), mod->get_var_count() * 100);
        std::optional<teddy::pla_file_binary> file = teddy::load_binary_pla(path, nullptr);
        teddy::bdd_manager::diagram_t f =
            teddy::io::from_pla(manager, *file)[mod->get_function_column()];
        std::vector<double> ps = manager.calculate_probabilities(*mod->get_sons_reliability(), f);
        mod->set_my_reliability(&ps);

        // teddy::ifmss_manager<2>::diagram_t f2 =
        //     ifmssManager.from_pla(*plaFile, teddy::fold_type::Left)[mod->get_function_column()];
        // ifmssManager.from_pla(const pla_file &file)
        // teddy::imss_manager::diagram_t f2 =
        // /*ifmssManager.from_pla(*plaFile)[mod->get_function_column()];*/
        // ifmssManager.from_pla(*plaFile, teddy::fold_type::Left)[mod->get_function_column()];

        // std::vector<double> ps =
        //     ifmssManager.calculate_probabilities(*mod->get_sons_reliability(), f2);
        // for (int i = 0; i < mod->get_states(); i++) {
        //     double prob = ps[i];
        //     mod->set_my_reliability(i, prob);
        // }
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
        pair.second->print_all();
    }
}
