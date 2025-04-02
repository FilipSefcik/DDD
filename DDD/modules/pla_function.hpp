#pragma once
#include <iostream>
#include <string>
#include <vector>

class pla_function {
  private:
    std::vector<std::string>* variables;
    std::vector<int>* fun_values;

  public:
    pla_function();
    ~pla_function();
    std::vector<std::string>* get_vars_from_fun(int function_value);
    std::vector<std::vector<std::string>*>* sort_by_function();
    std::vector<std::vector<std::string>*>* sort_by_position(int position);
    std::string replace_char(std::string before, int position, std::string input);
    int get_fun_value(std::string indexed_variables);

    void add_line(std::string new_vars, int value);

    void add_fun_value(int value) { this->fun_values->push_back(value); };
    void add_variables(std::string new_vars) { this->variables->push_back(new_vars); };

    void input_variables(std::vector<std::vector<std::string>*>* additional_vars, int position,
                         int function);

    void print_function();
};