#pragma once
#include "../modules/module_info.hpp"
#include <sstream>
#include <vector>

/**
 * @brief Manages all modules loaded from a PLA file.
 */
class module_manager {
  private:
    std::vector<module_info*>* modules_;
    std::vector<std::stringstream*>* separate_instructions_;

  public:
    module_manager();
    ~module_manager();

    // getters
    std::vector<module_info*>* get_modules() { return this->modules_; }
    int get_modules_count() { return this->modules_->size(); }

    // special functions
    void get_instructions(int processCount);
    std::string get_instructions_for_process(int processRank);

    // loading of modules
    void load(std::string confPath);
    void load_modules(std::string confPath);

    // info prints used only for troubleshooting
    void print_modules();
    void print_assigned_processes();
    void print_separate_instructions();
};
