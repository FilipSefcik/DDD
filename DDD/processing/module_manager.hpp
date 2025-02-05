#pragma once
#include "../modules/module_info.hpp"
#include "../utils/mpi_communicator.hpp"
#include <vector>

/**
 * @brief Manages all modules loaded from a PLA file.
 */
class module_manager {
  private:
    std::vector<module_info*>* modules_;
    std::vector<std::string>* separate_instructions_;

  public:
    module_manager();
    ~module_manager();

    // getters
    std::vector<module_info*>* get_modules() { return this->modules_; }
    int get_modules_count() { return this->modules_->size(); }

    // special functions
    void get_instructions(size_t processCount, void (*addInstruction)(module_info* mod, std::string* instructions));
    std::string get_instructions_for_process(size_t processRank);

    void create_messages(int numProcesses, std::vector<mpi_communicator::mpi_message>& messages);

    // loading of modules
    void load(std::string confPath);
    void load_modules(std::string confPath);

    // info prints used only for troubleshooting
    void print_modules();
    void print_assigned_processes();
    void print_separate_instructions();
};
