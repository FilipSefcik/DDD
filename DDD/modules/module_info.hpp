#pragma once
#include <iostream>
#include <string>
#include <vector>

class module_info {
  private:
    module_info* parent_ = nullptr;
    std::vector<module_info*> sons_;
    std::string name_;
    std::string pla_path_;
    std::vector<int> sons_states_;
    int function_column_ = 0;
    int states_ = 0;
    int priority_ = 0;
    int assigned_process_ = 0;
    int position_ = 0;
    int son_count_ = 0;
    std::vector<double>* my_reliabilities_ = nullptr;
    std::vector<std::vector<double>>* sons_reliability_ = nullptr;

  public:
    ~module_info() {
        delete this->sons_reliability_;
        delete this->my_reliabilities_;
        this->sons_reliability_ = nullptr;
        this->my_reliabilities_ = nullptr;
    }
    // Getters
    module_info* get_parent() const { return this->parent_; }
    const std::string& get_name() const { return this->name_; }
    const std::string& get_pla_path() const { return this->pla_path_; }
    int get_var_count() const { return this->sons_reliability_->size(); }
    int get_function_column() const { return this->function_column_; }
    int get_states() const { return this->states_; }
    int get_priority() const { return this->priority_; }
    int get_assigned_process() const { return this->assigned_process_; }
    int get_position() const { return this->position_; }
    int get_son_count() const { return this->son_count_; }

    std::vector<std::vector<double>>* get_sons_reliability() { return this->sons_reliability_; }

    // Setters
    void set_parent(module_info* parent) { this->parent_ = parent; }
    void set_name(const std::string& name) { this->name_ = name; }
    void set_pla_path(const std::string& plaPath) { this->pla_path_ = plaPath; }
    void set_function_column(int functionColumn) { this->function_column_ = functionColumn; }
    void set_states(int states) {
        this->states_ = states;
        this->my_reliabilities_ =
            new std::vector<double>(this->states_, 1.0 / this->states_); // Default reliability
    }
    void set_priority(int priority) { this->priority_ = priority; }
    void set_assigned_process(int assignedProcess) { this->assigned_process_ = assignedProcess; }
    void set_position(int position) { this->position_ = position; }
    void set_sons_domains(std::vector<int>* domains);

    void set_my_reliability(std::vector<double>* rel);
    void set_sons_reliability(size_t sonPosition, double sonRel, int state);
    void set_sons_reliability(size_t sonPosition, std::vector<double>* sonRel);
    void set_sons_reliability(std::vector<int>* domains);

    void add_son(int sonStates) { this->sons_states_.push_back(sonStates); }
    void add_module(module_info* newModule);
    void add_priority(int sonPriority);
    std::string to_string() const;

    void print_reliability(int state) {
        std::cout << "Module: " << this->name_ << std::endl;
        std::cout << "Density of " << state << ": " << this->my_reliabilities_->at(state)
                  << std::endl;
    };

    void print_sons();
};
