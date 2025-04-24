#include "module_info.hpp"
#include <iostream>
#include <sstream>

void module_info::add_module(module_info* newModule) {
    this->son_count_++;
    newModule->set_parent(this);
    this->sons_.push_back(newModule);
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
    std::cout << "Var count: " << this->sons_states_.size() << std::endl;
    std::cout << "Priority: " << this->priority_ << std::endl;
    std::cout << "Function column: " << this->function_column_ << std::endl;
    for (size_t i = 0; i < this->sons_states_.size(); i++) {
        std::cout << this->sons_states_.at(i) << " ";
    }
}

void module_info::set_my_reliability(std::vector<double>* rel) {
    this->my_reliabilities_->clear();
    // std::cout << this->states_ << std::endl;
    this->my_reliabilities_->resize(this->states_);
    for (int i = 0; i < this->states_; i++) {
        this->my_reliabilities_->at(i) = rel->at(i);
    }
}

void module_info::set_sons_domains(std::vector<int>* domains) {
    if (! this->sons_reliability_) {
        this->sons_reliability_ = new std::vector<std::vector<double>>();
    } else {
        this->sons_reliability_->clear();
    }
    for (int i = 0; i < domains->at(0); i++) {
        this->sons_reliability_->push_back(std::vector<double>(2, 1.0 / 2));
    }
    for (size_t i = 1; i < domains->size(); i++) {
        this->sons_reliability_->push_back(std::vector<double>(domains->at(i), 1.0 / domains->at(i)));
    }
}

void module_info::set_sons_reliability(size_t sonPosition, double sonRel, int state) {
    if (sonPosition < this->sons_reliability_->size()) {
        // std::cout << sonRel << std::endl;
        this->sons_reliability_->at(sonPosition).at(state) = sonRel;
        this->sons_reliability_->at(sonPosition).at((state + 1) % 2) = 1.0 - sonRel;
    }
}

void module_info::set_sons_reliability(size_t sonPosition, std::vector<double>* sonRel) {
    this->sons_reliability_->at(sonPosition) = *sonRel;
}

void module_info::set_sons_reliability(std::vector<int>* domains) {
    this->sons_reliability_->clear();
    this->sons_reliability_->resize(domains->size());
    for (size_t i = 0; i < domains->size(); i++) {
        // std::cout << domains->at(i) << std::endl;
        this->sons_reliability_->at(i).resize(domains->at(i), 1.0 / domains->at(i));
    }
}