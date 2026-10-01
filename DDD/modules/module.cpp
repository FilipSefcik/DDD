#include "module.hpp"
#include "pla_function.hpp"
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <limits>
#include <ostream>
#include <sstream>
#include <stdexcept>

module::module(const std::string& paName, int paStates) {
    this->name_ = paName;
    this->states_ = paStates;
    this->sons_reliability_ = new std::vector<std::vector<double>>();
    this->my_reliabilities_ = new std::vector<double>(this->states_);
    this->sons_map_ = new std::unordered_map<std::string, int>();
    this->derivatives_ = new std::vector<std::vector<double>>(this->states_ - 1);
    this->son_derivatives_ = new std::vector<std::vector<double>>();
}

module::~module() {
    delete this->derivatives_;
    delete this->son_derivatives_;
    delete this->sons_reliability_;
    delete this->my_reliabilities_;
    delete this->sons_rel_count_;
    delete this->function_;
    delete this->sons_map_;
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
    fields >> this->function_column_ >> this->position_ >> this->states_ >> this->var_count_ >>
        this->start_index_ >> this->end_index_;

    // Initialize reliabilities
    this->my_reliabilities_ =
        new std::vector<double>(this->states_, 1.0 / this->states_); // Default reliability
    this->sons_reliability_ = new std::vector<std::vector<double>>(this->var_count_);
    this->sons_rel_count_ = new std::vector<int>(this->var_count_);
    this->derivatives_ = new std::vector<std::vector<double>>(this->states_ - 1);
    this->son_derivatives_ = new std::vector<std::vector<double>>();

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

void module::insert_function_in_parallel(pla_function* otherFunction, std::string sonName,
                                         int numOfParts) {
    if (this->function_ && otherFunction) {
        int sonPosition = this->sons_map_->at(sonName);
        this->function_->input_variables_in_parallel(otherFunction, sonPosition, numOfParts);
        for (auto& pair : *this->sons_map_) {
            if (sonPosition < pair.second) {
                pair.second += otherFunction->get_var_count() - 1;
            }
        }
    }
}

void module::set_var_count(int paVarCount) {
    this->var_count_ = paVarCount;
}

void module::set_derivatives(const std::vector<std::vector<double>>* paDeriv) {
    if (paDeriv == nullptr) {
        throw std::invalid_argument("Cannot set derivatives from a null pointer");
    }
    if (this->derivatives_ == nullptr) {
        this->derivatives_ = new std::vector<std::vector<double>>();
    }
    *this->derivatives_ = *paDeriv;
}

void module::set_son_derivatives(const std::vector<std::vector<double>>* paDeriv) {
    if (paDeriv == nullptr) {
        throw std::invalid_argument("Cannot set son derivatives from a null pointer");
    }
    if (this->son_derivatives_ == nullptr) {
        this->son_derivatives_ = new std::vector<std::vector<double>>();
    }
    *this->son_derivatives_ = *paDeriv;
}

void module::set_sons_reliability(size_t sonPosition, double sonRel, int state) {
    if (sonPosition < this->sons_reliability_->size()) {
        // std::cout << sonRel << std::endl;
        this->sons_reliability_->at(sonPosition).at(state) = sonRel;
        this->sons_reliability_->at(sonPosition).at((state + 1) % 2) = 1.0 - sonRel;
    }
}

void module::set_sons_reliability(size_t sonPosition, std::vector<double>&& sonRel) {
    (*sons_reliability_)[sonPosition] = sonRel;
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

std::string module::get_derivatives_as_string() {
    std::string result;
    if (this->derivatives_ == nullptr) {
        return result;
    }
    for (const auto& deriv : *this->derivatives_) {
        for (double val : deriv) {
            result += std::to_string(val) + " ";
        }
        result += "\n";
    }
    return result;
}

std::string module::get_son_derivatives_as_string() {
    std::string result;
    if (this->son_derivatives_ == nullptr) {
        return result;
    }
    for (const auto& deriv : *this->son_derivatives_) {
        for (double val : deriv) {
            result += std::to_string(val) + " ";
        }
        result += "\n";
    }
    return result;
}

std::string module::get_derivative_matrix() {
    std::ostringstream result;
    if (this->derivatives_ == nullptr) {
        return result.str();
    }

    size_t column_count = 0;
    for (const auto& row : *this->derivatives_) {
        if (row.size() > column_count) {
            column_count = row.size();
        }
    }

    const auto format_value = [](double value) {
        std::ostringstream value_stream;
        value_stream << std::setprecision(std::numeric_limits<double>::max_digits10) << value;
        return value_stream.str();
    };

    size_t cell_width = std::string("S/C").size();
    for (size_t j = 0; j < column_count; ++j) {
        cell_width = std::max(cell_width, std::to_string(j).size());
    }
    for (size_t i = 0; i < this->derivatives_->size(); ++i) {
        cell_width = std::max(cell_width, std::to_string(i).size());
        for (const double value : this->derivatives_->at(i)) {
            cell_width = std::max(cell_width, format_value(value).size());
        }
    }

    result << "S: " << this->derivatives_->size() << ", C: " << column_count << "\n";
    result << std::setw(cell_width) << "S/C";
    for (size_t j = 0; j < column_count; ++j) {
        result << " | " << std::setw(cell_width) << j;
    }
    result << "\n";

    for (size_t i = 0; i < this->derivatives_->size(); ++i) {
        result << std::setw(cell_width) << i;
        for (size_t j = 0; j < column_count; ++j) {
            result << " | " << std::setw(cell_width);
            if (j < this->derivatives_->at(i).size()) {
                result << format_value(this->derivatives_->at(i).at(j));
            }
        }
        result << "\n";
    }
    return result.str();
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
    std::cout << "Start index: " << this->start_index_ << std::endl;
    std::cout << "End index: " << this->end_index_ << std::endl;
    std::cout << "States: " << this->states_ << std::endl;
    std::cout << "My reliabilities:\n";
    print_reliabilities();
    std::cout << "Sons reliabilities: \n";
    print_sons_reliabilities();
    std::cout << "Sons map: \n";
    print_son_map();
    std::cout << "Derivatives: \n";
    std::cout << get_derivatives_as_string() << std::endl;
    std::cout << "Son derivatives: \n";
    std::cout << get_son_derivatives_as_string() << std::endl;
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
