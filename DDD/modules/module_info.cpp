#include "module_info.hpp"

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

void module_info::print_sons() {
    if (this->parent_) {
        std::cout << "Parent: " << this->parent_->get_name() << std::endl;
    }
    std::cout << "Var count: " << this->sons_states_.size() << std::endl;
    std::cout << "Priority: " << this->priority_ << std::endl;
    std::cout << "Function column: " << this->function_column_ << std::endl;
    for (int i = 0; i < this->sons_states_.size(); i++) {
        std::cout << this->sons_states_.at(i) << " ";
    }
}