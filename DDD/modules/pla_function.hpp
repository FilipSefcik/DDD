#pragma once
#include <iostream>
#include <string>
#include <vector>

class pla_function {
  private:
    std::vector<std::string>* variables_;
    std::vector<int>* fun_values_;

  public:
    pla_function();
    ~pla_function();
    void load_from_pla(const std::string& filePath);
    std::vector<std::string>* get_vars_from_fun(int functionValue);
    std::vector<std::vector<std::string>*>* sort_by_function();
    std::vector<std::vector<std::string>*>* sort_by_position(int position);
    std::string replace_char(std::string before, int position, std::string input);
    int get_fun_value(std::string indexedVariables);

    void add_line(std::string newVars, int value);

    void add_fun_value(int value) { this->fun_values_->push_back(value); };
    void add_variables(std::string newVars) { this->variables_->push_back(newVars); };

    void input_variables(std::vector<std::vector<std::string>*>* additionalVars, int position,
                         int function);

    void print_function();
};