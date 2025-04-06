#pragma once
#include "pla_function.hpp"
#include <string>
#include <vector>

/**
 * @brief Class module describes a module in config file
 *
 * It's a binary function described by a PLA file, but
 * each variable can have different probabilities of 1 and 0.
 */
class module {
  private:
    std::string name_, path_;
    int position_ = 0;
    int var_count_ = 0;
    int function_column_ = 0;
    int states_ = 0;
    std::vector<double>* my_reliabilities_ = nullptr;
    std::vector<std::vector<double>>* sons_reliability_ = nullptr;
    std::vector<int>* sons_rel_count_ = nullptr;
    pla_function* function_ = nullptr;

  public:
    module(const std::string& paName, int paStates);
    module(std::string infoToString);
    ~module() {
        delete this->sons_reliability_;
        delete this->my_reliabilities_;
        delete this->sons_rel_count_;
        delete this->function_;
        this->sons_reliability_ = nullptr;
        this->my_reliabilities_ = nullptr;
        this->sons_rel_count_ = nullptr;
    }

    // getters
    std::string get_name() { return this->name_; }
    std::string get_path() { return this->path_; }

    std::vector<std::vector<double>>* get_sons_reliability() { return this->sons_reliability_; }
    std::vector<int>* get_son_rel_count() { return this->sons_rel_count_; }
    int get_position() { return this->position_; }
    int get_var_count() { return this->var_count_; }
    int get_function_column() { return this->function_column_; }
    int get_states() { return this->states_; }

    double get_reliability(int state) { return this->my_reliabilities_->at(state); }
    std::vector<double>* get_my_reliabilities() { return this->my_reliabilities_; }

    // setters

    void set_path(const std::string& paPath) { this->path_ = paPath; }

    void set_sons_reliability(size_t sonPosition, double sonRel, int state);

    void set_position(int paPosition) { this->position_ = paPosition; }
    void set_var_count(int paVarCount);
    void set_function_column(int paColumn) { this->function_column_ = paColumn; }

    void set_sons_reliability(size_t sonPosition, std::vector<double>* sonRel);
    void set_sons_reliability(std::vector<int>* domains);
    void set_my_reliability(std::vector<double>* rel);
    void set_my_reliability(int state, double rel) { this->my_reliabilities_->at(state) = rel; }

    // prints used to get info
    // used only during troubleshooting
    void print_all();
    void print_sons_reliabilities();
    void print_reliabilities();
};
