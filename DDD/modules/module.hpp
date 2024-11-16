#pragma once
#include <string>
#include <unordered_map>
#include <vector>

/*
 * @brief Class module describes a module in config file
 *
 * It's a binary function described by a PLA file, but
 * each variable can have different probabilities of 1 and 0.
 */
class module {
  private:
    std::string name_, path_, pla_file_;
    std::vector<std::vector<double>>* sons_reliability_;
    module* parent_ = nullptr;
    int assigned_process_ = 0;
    int position_ = 0;
    int priority_ = 0; // number of modules between the furthest "child" module and this module
    int var_count_ = 0;
    int function_column_ = 0;
    std::unordered_map<std::string, int> sons_map_;

    int states_;
    std::vector<double>* my_reliabilities_;

  public:
    module(std::string paName, int paStates);
    ~module() {
        delete this->sons_reliability_;
        delete this->my_reliabilities_;
    }

    // getters

    module* get_parent() { return this->parent_; }

    std::string get_name() { return this->name_; }
    std::string get_path() { return this->path_; }
    std::string get_pla() { return this->pla_file_; }

    std::vector<std::vector<double>>* get_sons_reliability() { return this->sons_reliability_; }

    int get_priority() { return this->priority_; }
    int get_process_rank() { return this->assigned_process_; }
    int get_position() { return this->position_; }
    int get_var_count() { return this->var_count_; }
    int get_function_column() { return this->function_column_; }

    double get_reliability(int state) { return this->my_reliabilities_->at(state); }
    std::vector<double>* get_my_reliabilities() { return this->my_reliabilities_; }

    // setters

    void set_parent(module* paParent) { this->parent_ = paParent; }

    void set_path(std::string paPath) { this->path_ = paPath; }
    void set_pla(std::string plaContent) { this->pla_file_ = plaContent; }

    void set_sons_reliability(int sonPosition, double sonRel, int state);

    void set_position(int paPosition) { this->position_ = paPosition; }
    void assign_process(int paProcess) { this->assigned_process_ = paProcess; }
    void set_var_count(int paVarCount);
    void set_function_column(int paColumn) { this->function_column_ = paColumn; }

    void set_sons_reliability(int sonPosition, std::vector<double>* sonRel);
    void set_my_reliability(std::vector<double>* rel) { *this->my_reliabilities_ = *rel; }

    // special functions

    void add_son(module* newSon);
    void add_priority(int sonPriority);

    // prints used to get info
    // used only during troubleshooting

    int get_son_position(std::string sonName) { return this->sons_map_.at(sonName); }
    void add_son_position(std::string sonName, int position) {
        this->sons_map_.emplace(sonName, position);
    }
    void edit_position(std::string sonName, int newPosition) {
        this->sons_map_.at(sonName) = newPosition;
    }

    void print_pla();
    void print_sons();
    void write_pla_file();
    void print_sons_reliabilities();
    void print_reliabilities();
};
