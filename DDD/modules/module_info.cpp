#include "module_info.hpp"
#include <iostream>
#include <sstream>

void module_info::add_module(module_info* newModule) {
    newModule->set_position(this->sons_states_.size());
    newModule->set_parent(this);
    this->add_son(newModule->get_states());
    this->add_priority(newModule->get_priority());
}

void module_info::add_priority(int sonPriority) {
    if (this->priority_ == sonPriority) {
        this->priority_++;
        if (this->parent_) {
            this->parent_->add_priority(this->priority_);
        }
    }
}

std::string module_info::to_string() const {
    std::stringstream data;
    data << this->name_ << " " << this->pla_path_ << " " << this->function_column_ << " "
         << this->states_ << " " << this->get_var_count();
    for (int state : this->sons_states_) {
        data << " ";
        data << state;
    }
    return data.str();
}

void module_info::print_sons() {
    if (this->parent_) {
        std::cout << "Parent: " << this->parent_->get_name() << std::endl;
    }
    std::cout << "Var count: " << this->sons_states_.size() << std::endl;
    std::cout << "Priority: " << this->priority_ << std::endl;
    std::cout << "Function column: " << this->function_column_ << std::endl;
    for (size_t i = 0; i < this->sons_states_.size(); i++) {
        std::cout << this->sons_states_.at(i) << " ";
    }
}

void module_info::set_sons_domains(std::vector<int>* domains) {
    for (int i = 0; i < domains->at(0); i++) {
        this->sons_states_.push_back(2);
    }
    for (size_t i = 1; i < domains->size(); i++) {
        this->sons_states_.push_back(domains->at(i));
    }
}
