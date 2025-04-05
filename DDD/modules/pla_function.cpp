#include "pla_function.hpp"
#include <cstddef>
#include <fstream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

pla_function::pla_function() {
    this->variables_ = new std::vector<std::string>();
    this->fun_values_ = new std::vector<int>();
}

pla_function::~pla_function() {
    delete this->variables_;
    delete this->fun_values_;
}

void pla_function::load_from_pla(const std::string& filePath) {

    std::ifstream inputFile(filePath);
    if (! inputFile.is_open()) {
        throw std::runtime_error("Error opening file: " + filePath);
    }

    std::string line;
    int varCount = 0;

    while (std::getline(inputFile, line)) {
        // Preskoč prázdne riadky, komentáre a príkazy začínajúce bodkou
        if (line.empty() || line[0] == '#') {
            continue;
        }

        if (line[0] == '.') {
            // Ak ide o riadok s počtom vstupov, napr. ".i 5"
            if (line.size() > 1 && line[1] == 'i') {
                std::istringstream iss(line);
                std::string token;
                iss >> token; // token by mal byť ".i"
                iss >> varCount; // načíta počet vstupných premenných
            }
            continue;
        }

        if (varCount == 0) {
            throw std::runtime_error("Input variable count not defined in file: " + filePath);
        }

        // Prvých var_count znakov predstavuje bitové slovo (premenné)
        std::string variables = line.substr(0, varCount);
        int value = std::stoi(line.substr(varCount + 1));

        // Pridaj načítaný riadok do PLA funkcie
        this->add_line(variables, value);
    }

    inputFile.close();
}

std::vector<std::string>* pla_function::get_vars_from_fun(int functionValue) {
    std::vector<std::string>* all_vars = new std::vector<std::string>;

    for (int i = 0; i < this->fun_values_->size(); i++) {
        if (this->fun_values_->at(i) == functionValue) {
            all_vars->push_back(this->variables_->at(i));
        }
    }

    return all_vars;
}

void pla_function::add_line(std::string newVars, int value) {
    this->add_variables(newVars);
    this->add_fun_value(value);
}

void pla_function::print_function() {
    std::cout << "variables       function\n";
    for (int i = 0; i < this->fun_values_->size(); i++) {
        std::cout << this->variables_->at(i) << " " << this->fun_values_->at(i) << std::endl;
    }
}

std::vector<std::vector<std::string>*>* pla_function::sort_by_function() {
    std::vector<std::vector<std::string>*>* sorted = new std::vector<std::vector<std::string>*>();
    for (int i = 0; i < 2; i++) {
        std::vector<std::string>* new_function = new std::vector<std::string>();
        for (int j = 0; j < this->fun_values_->size(); j++) {
            if (this->fun_values_->at(j) == i) {
                new_function->push_back(this->variables_->at(j));
            }
        }
        sorted->push_back(new_function);
    }
    return sorted;
}

std::vector<std::vector<std::string>*>* pla_function::sort_by_position(int position) {
    std::vector<std::vector<std::string>*>* sorted = new std::vector<std::vector<std::string>*>();
    std::vector<std::string>* whatever_lines = new std::vector<std::string>();
    for (int i = 0; i < 2; i++) {
        std::vector<std::string>* new_function = new std::vector<std::string>();
        for (int j = 0; j < this->variables_->size(); j++) {
            if (this->variables_->at(j)[position] == '-' && i == 0) {
                whatever_lines->push_back(this->variables_->at(j));
            }
            if (this->variables_->at(j)[position] - '0' == i) {
                new_function->push_back(this->variables_->at(j));
            }
        }
        sorted->push_back(new_function);
    }
    sorted->push_back(whatever_lines);
    return sorted;
}

void pla_function::input_variables(std::vector<std::vector<std::string>*>* additionalVars,
                                   int position, int function) {
    std::vector<std::string>* new_vars = new std::vector<std::string>();
    std::vector<int>* new_fun_vals = new std::vector<int>();

    std::vector<std::vector<std::string>*>* my_vars = this->sort_by_position(position);
    std::string temp_line, input_line;
    int var_count = additionalVars->at(0)->at(0).length();
    int fun_value;
    std::string whatever_input = std::string(var_count, '-');

    for (size_t i = 0; i < my_vars->size(); i++) {
        if (i == my_vars->size() - 1) {
            for (int l = 0; l < my_vars->at(i)->size(); l++) {

                temp_line = my_vars->at(i)->at(l);
                fun_value = this->get_fun_value(temp_line);

                temp_line = this->replace_char(temp_line, position, whatever_input);

                new_vars->push_back(temp_line);
                new_fun_vals->push_back(fun_value);
            }
            continue;
        }
        for (int j = 0; j < my_vars->at(i)->size(); j++) {
            temp_line = my_vars->at(i)->at(j);
            fun_value = this->get_fun_value(temp_line);
            for (int k = 0; k < additionalVars->at(i)->size(); k++) {
                temp_line = my_vars->at(i)->at(j);
                input_line = additionalVars->at(i)->at(k);

                temp_line = this->replace_char(temp_line, position, input_line);

                new_vars->push_back(temp_line);
                new_fun_vals->push_back(fun_value);
            }
        }
    }

    this->variables_ = new_vars;
    this->fun_values_ = new_fun_vals;
}

int pla_function::get_fun_value(std::string indexedVariables) {
    // Find the iterator to the element
    auto it = std::find(this->variables_->begin(), this->variables_->end(), indexedVariables);

    int value = 0;

    // Check if the element was found
    if (it != this->variables_->end()) {
        // Calculate the index
        int index = std::distance(this->variables_->begin(), it);
        value = this->fun_values_->at(index);
    }

    return value;
}

std::string pla_function::replace_char(std::string before, int position, std::string input) {
    std::string before_part = before.substr(0, position);
    std::string after_part = before.substr(position + 1);
    return before_part + input + after_part;
}