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
    void (*execute_module_)(mpi_manager* manager, const std::string& inputString);
    std::string (*serialize_module_)(mpi_manager* manager, const std::string& inputString);
    void (*deserialize_module_)(mpi_manager* manager, const std::string& parameter,
                                const std::string& inputString);

  public:
    mpi_manager(std::string moduleData);
    ~mpi_manager();

    void set_function(void (*executeModule)(mpi_manager* manager, const std::string& inputString)) {
        this->execute_module_ = executeModule;
    }

    void set_serialize_function(std::string (*serializeModule)(mpi_manager* manager,
                                                               const std::string& inputString)) {
        this->serialize_module_ = serializeModule;
    }

    void set_deserialize_function(void (*deserializeModule)(mpi_manager* manager,
                                                            const std::string& parameter,
                                                            const std::string& inputString)) {
        this->deserialize_module_ = deserializeModule;
    }

    std::unordered_map<std::string, module*> get_my_modules() { return this->my_modules_; }
    void add_module(module* mod) { this->my_modules_.insert({mod->get_name(), mod}); }
    int get_calculated_state() const { return this->calculated_state_; }

    // Instructions as functions
    void complete_instructions(const std::string& instructions, int state);
    // void evaluate(const std::string& moduleName);
    void send_module(const std::string& parameter, int receiversRank);
    void recv_module(const std::string& parameter, int sender);

    // Used for testing
    void print_my_modules(int myRank);
};
