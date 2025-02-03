#pragma once
#include "../modules/module.hpp"
#include <libteddy/impl/diagram_manager.hpp>
#include <libteddy/inc/reliability.hpp>

/**
 * @brief Completes instructions from the mpi_manager class and communicates using mpi_communicator.
 */
class mpi_manager {
  private:
    std::unordered_map<std::string, module*> my_modules_;
    int calculated_state_ = 0;

  public:
    mpi_manager(std::string moduleData);
    ~mpi_manager();

    // Instructions as functions
    void complete_instructions(std::string instructions, int state);
    void evaluate(std::string moduleName);
    void execute_module(std::string moduleName, int modulePosition);
    void link_modules(std::string parentName, std::string sonName);
    void send_module(std::string moduleName, int receiversRank);
    void recv_module(std::string parentName, int sender);

    // Used for testing
    void print_my_modules(int myRank);
};
