#include "module.hpp"
#include <iostream>

module::module(std::string paName, int paStates) {
    this->name_ = paName;
    this->states_ = paStates;
    this->sons_reliability_ = new std::vector<std::vector<double>>();
    this->my_reliabilities_ = new std::vector<double>(this->states_);
}

/**
 * @brief Sets this module as a parent for the new module in parameter and calculates the overall
 * priorities.
 * @param newSon New module, which is "child" of this module.
 */
void module::add_son(module* newSon) {
    newSon->set_parent(this);
    this->add_priority(newSon->get_priority());
}

/**
 * @brief Calculates new priorities for all modules directly above this module, including this
 * module.
 * @param sonPriority Priority of a son module.
 */
void module::add_priority(int sonPriority) {
    if (this->priority_ == sonPriority) {
        this->priority_++;
        if (this->parent_) {
            this->parent_->add_priority(this->priority_);
        }
    }
}

void module::set_var_count(int paVarCount) {
    this->var_count_ = paVarCount;
    for (int i = 0; i < this->var_count_; i++) {
        this->sons_reliability_->push_back(std::vector<double>(this->states_, 1.0 / this->states_));
    }
}

void module::set_sons_reliability(int sonPosition, double sonRel, int state) {
    if (sonPosition >= 0 && sonPosition < this->sons_reliability_->size()) {
        std::cout << sonRel << std::endl;
        this->sons_reliability_->at(sonPosition).at(state) = sonRel;
        this->sons_reliability_->at(sonPosition).at((state + 1) % 2) = 1.0 - sonRel;
    }
}

void module::set_sons_reliability(int sonPosition, std::vector<double>* sonRel) {
    this->sons_reliability_->at(sonPosition) = *sonRel;
}

void module::print_sons_reliabilities() {
    for (int i = 0; i < this->sons_reliability_->size(); i++) {
        for (int j = 0; j < this->sons_reliability_->at(i).size(); j++) {
            std::cout << this->sons_reliability_->at(i).at(j) << " ";
        }
        std::cout << std::endl;
    }
}

void module::print_sons() {
    std::cout << "Var count: " << this->var_count_ << std::endl;
    std::cout << "Priority: " << this->priority_ << std::endl;

    for (int i = 0; i < this->sons_reliability_->size(); i++) {
        for (int j = 0; j < this->sons_reliability_->at(i).size(); j++) {
            std::cout << this->sons_reliability_->at(i).at(j) << " ";
        }
        std::cout << std::endl;
    }
}

void module::print_reliabilities() {
    for (double prob : *this->my_reliabilities_) {
        std::cout << prob << " ";
    }
    std::cout << std::endl;
}
