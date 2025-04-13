#include "module.hpp"
#include <cstdio>
#include <iostream>
#include <ostream>
#include <sstream>

module::module(const std::string& paName, int paStates) {
    this->name_ = paName;
    this->states_ = paStates;
    this->sons_reliability_ = new std::vector<std::vector<double>>();
    this->my_reliabilities_ = new std::vector<double>(this->states_);
    this->sons_map_ = new std::unordered_map<std::string, int>();
}

module::module(std::string infoToString) {
    size_t plaIndex = infoToString.find(".pla");
    if (plaIndex == std::string::npos) {
        throw std::invalid_argument("Invalid input string: .pla not found");
    }

    // Extract the module name
    std::stringstream ss(infoToString);
    ss >> this->name_;

    // Extract the path (including spaces)
    size_t pathStart =
        infoToString.find(this->name_) + this->name_.size() + 1; // Start after the name and space
    this->path_ = infoToString.substr(pathStart, plaIndex + 4 - pathStart);

    // Move to the fields after the path
    size_t afterPathIndex = plaIndex + 5; // ".pla" + space
    std::stringstream fields(infoToString.substr(afterPathIndex));
    fields >> this->function_column_ >> this->position_ >> this->states_ >> this->var_count_;

    // Initialize reliabilities
    this->my_reliabilities_ =
        new std::vector<double>(this->states_, 1.0 / this->states_); // Default reliability
    this->sons_reliability_ = new std::vector<std::vector<double>>(this->var_count_);
    this->sons_rel_count_ = new std::vector<int>(this->var_count_);

    // Reading son states into sons_reliability_
    for (int i = 0; i < this->var_count_; ++i) {
        int sonState;
        fields >> sonState;
        this->sons_reliability_->at(i) = std::vector<double>(sonState, 1.0 / sonState);
        this->sons_rel_count_->at(i) = (sonState);
    }

    fields >> this->son_count_;

    this->sons_map_ = new std::unordered_map<std::string, int>();

    for (int i = 0; i < this->son_count_; ++i) {
        std::string sonName;
        int sonPosition;
        fields >> sonName >> sonPosition;
        this->sons_map_->insert({sonName, sonPosition});
    }
}

void module::insert_function(pla_function* otherFunction, std::string sonName) {
    if (this->function_ && otherFunction) {
        int sonPosition = this->sons_map_->at(sonName);
        this->function_->input_variables(otherFunction, sonPosition);
        for (auto& pair : *this->sons_map_) {
            if (sonPosition < pair.second) {
                pair.second += otherFunction->get_var_count() - 1;
            }
        }
    }
}

void module::insert_function(char*** additionalVars, int otherVarCount, const int* otherFunValCount,
                             std::string sonName) {
    if (this->function_) {
        int sonPosition = this->sons_map_->at(sonName);
        this->function_->input_variables(additionalVars, otherVarCount, otherFunValCount,
                                         sonPosition);
        for (auto& pair : *this->sons_map_) {
            if (sonPosition < pair.second) {
                pair.second += otherVarCount - 1;
            }
        }
    }
}

void module::set_var_count(int paVarCount) {
    this->var_count_ = paVarCount;
}

void module::set_sons_reliability(size_t sonPosition, double sonRel, int state) {
    if (sonPosition < this->sons_reliability_->size()) {
        // std::cout << sonRel << std::endl;
        this->sons_reliability_->at(sonPosition).at(state) = sonRel;
        this->sons_reliability_->at(sonPosition).at((state + 1) % 2) = 1.0 - sonRel;
    }
}

void module::set_sons_reliability(size_t sonPosition, std::vector<double>* sonRel) {
    this->sons_reliability_->at(sonPosition) = *sonRel;
}

void module::set_sons_reliability(std::vector<int>* domains) {
    this->sons_reliability_->clear();
    this->sons_reliability_->resize(domains->size());
    this->sons_rel_count_->resize(domains->size());
    for (size_t i = 0; i < domains->size(); i++) {
        // std::cout << domains->at(i) << std::endl;
        this->sons_reliability_->at(i).resize(domains->at(i), 1.0 / domains->at(i));
        this->sons_rel_count_->at(i) = domains->at(i);
    }
}

void module::set_my_reliability(std::vector<double>* rel) {
    this->my_reliabilities_->clear();
    // std::cout << this->states_ << std::endl;
    this->my_reliabilities_->resize(this->states_);
    for (int i = 0; i < this->states_; i++) {
        this->my_reliabilities_->at(i) = rel->at(i);
    }
}

void module::print_sons_reliabilities() {
    std::cout << "-------------------------\n";
    std::cout << this->get_name() << " Sons reliabilities: \n";
    for (size_t i = 0; i < this->sons_reliability_->size(); i++) {
        std::cout << this->sons_rel_count_->at(i) << " ";
        for (size_t j = 0; j < this->sons_reliability_->at(i).size(); j++) {
            std::cout << this->sons_reliability_->at(i).at(j) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << "-------------------------\n";
}

void module::print_all() {
    std::cout << "Name: " << this->name_ << std::endl;
    std::cout << "Pla path: " << this->path_ << std::endl;
    std::cout << "Var count: " << this->var_count_ << std::endl;
    std::cout << "Position: " << this->position_ << std::endl;
    std::cout << "Function column: " << this->function_column_ << std::endl;
    // std::cout << "My reliabilities:\n";
    // print_reliabilities();
    // std::cout << "Sons reliabilities: \n";
    // print_sons_reliabilities();
}

void module::print_reliabilities() {
    std::cout << "-------------------------\n";
    std::cout << this->get_name() << " Reliabilities: \n";
    double sum = 0;
    std::cout << this->my_reliabilities_->size() << "\n";
    for (double prob : *this->my_reliabilities_) {
        sum += prob;
        std::cout << prob << " ";
    }
    std::cout << "Sum: " << sum << std::endl;
    std::cout << std::endl;
    std::cout << "-------------------------\n";
}

void module::print_son_map() {
    std::cout << "-------------------------\n";
    std::cout << this->get_name() << " Sons map: \n";
    for (const auto& pair : *this->sons_map_) {
        std::cout << pair.first << " " << pair.second << std::endl;
    }
    std::cout << std::endl;
    std::cout << "-------------------------\n";
}
