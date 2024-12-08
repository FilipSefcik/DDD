#pragma once
#include "../modules/module.hpp"
#include "../utils/mpi_communicator.hpp"
#include <cstdint>
#include <libteddy/details/diagram_manager.hpp>
#include <libteddy/reliability.hpp>

/*
 * @brief completes instructions from class mpi_manager and communicates using mpi_communicator
 */
class mpi_manager {
  private:
    std::unordered_map<std::string, module*> my_modules;
    int calculated_state = 0;

  public:
    ~mpi_manager();

    // instructions as functions

    void complete_instructions(std::string instructions, int state);
    void evaluate(std::string module_name);
    void execute_module(std::string module_name, int module_position);
    void link_modules(std::string parent_name, std::string son_name);
    void send_module(std::string module_name, int recievers_rank);
    void recv_module(std::string parent_name, int sender);

    // communicating with modules

    void recieve_my_modules(int pa_my_assigned_modules, int pa_my_rank,
                            std::string& pa_my_instructions);
    void add_new_module(std::string name, std::string pla, int my_rank, int var_count,
                        int function_column);
    void send_module_info(module* mod, /*std::string instructions,*/ int recievers_rank);

    // processing modules

    void write_to_pla();

    // used for testing

    void print_my_modules(int my_rank);
};