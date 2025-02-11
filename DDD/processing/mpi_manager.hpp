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
    void (*execute_module_)(mpi_manager* manager, std::string inputString);

  public:
    mpi_manager(std::string moduleData);
    ~mpi_manager();

    void set_function(void (*executeModule)(mpi_manager* manager,
                                            std::string inputString)) {
        this->execute_module_ = executeModule;
    }

    std::unordered_map<std::string, module*> get_my_modules() { return this->my_modules_; }

    // Instructions as functions
    void complete_instructions(std::string instructions, int state);
    void evaluate(std::string moduleName);
    void send_module(std::string moduleName, int receiversRank);
    void recv_module(std::string parentName, int sender);

    // Used for testing
    void print_my_modules(int myRank);
};
