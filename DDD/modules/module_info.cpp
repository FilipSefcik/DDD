#include "module_info.hpp"
#include <iostream>
#include <sstream>

void module_info::add_module(module_info* newModule) {
    this->son_count_++;
    newModule->set_parent(this);
    this->sons_.push_back(newModule);
    this->add_priority(newModule->get_priority());

    for (module_info* son : this->sons_) {
        if (son->get_name() == newModule->get_name()) {
            break;
        }
        newModule->set_offset_start(newModule->get_offset_start() + son->get_var_count() - 1);
        newModule->set_offset_end(newModule->get_offset_end() + son->get_var_count() - 1);
    }
    this->add_offset_end(newModule, newModule->get_var_count());
}

void module_info::add_priority(int sonPriority) {
    if (this->priority_ == sonPriority) {
        this->priority_++;
        if (this->parent_) {
            this->parent_->add_priority(this->priority_);
        }
    }
}

void module_info::add_offset_end(module_info* newModule, int additionalVars) {
    int surplus = additionalVars - 1;
    this->offset_end_ += surplus;
    bool add = false;
    for (module_info* son : this->sons_) {
        if (add) {
            son->add_offset_start_and_end(additionalVars);
            break;
        }
        if (son->get_name() == newModule->get_name()) {
            add = true;
        }
    }
    if (this->parent_) {
        this->parent_->add_offset_end(this, additionalVars);
    }
}

void module_info::add_offset_start_and_end(int additionalVars) {

    int surplus = additionalVars - 1;
    this->offset_start_ += surplus;
    this->offset_end_ += surplus;

    for (module_info* son : this->sons_) {
        son->add_offset_start_and_end(additionalVars);
    }
}

std::string module_info::to_string() const {
    std::stringstream data;
    data << this->name_ << " " << this->pla_path_ << " " << this->function_column_ << " "
         << this->position_ << " " << this->states_ << " " << this->get_var_count();
    for (int state : this->sons_states_) {
        data << " ";
        data << state;
    }

    data << " " << this->sons_.size();

    for (const auto& son : this->sons_) {
        data << " " << son->get_name();
        data << " " << son->get_position();
    }

    return data.str();
}

void module_info::print_sons() {
    if (this->parent_) {
        std::cout << "Parent: " << this->parent_->get_name() << std::endl;
    }
    std::cout << "Sons: ";
    for (size_t i = 0; i < this->sons_.size(); i++) {
        std::cout << this->sons_.at(i)->get_name() << " ";
    }
    std::cout << std::endl;
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
