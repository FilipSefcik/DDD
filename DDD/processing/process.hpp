#pragma once
#include "../utils/mpi_communicator.hpp"
#include "module_manager.hpp"
#include "mpi_manager.hpp"
#include <string>

/**
 * @brief Describes basic functions of any process.
 */
class process {
  protected:
    mpi_manager* mpi_manager_ = nullptr;
    std::string my_instructions_;
    int my_rank_ = -1;

    void deserialize_message(mpi_communicator::mpi_message message);

  public:
    process(int rank) : my_rank_(rank){};
    virtual ~process();
    virtual void process_information() = 0;
    void process_instructions(int state);
    void set_function(void (*executeModule)(mpi_manager* manager, const std::string& inputString)) {
        if (this->mpi_manager_) {
            this->mpi_manager_->set_function(executeModule);
        }
    }
    void set_serialize_function(std::string (*serializeModule)(mpi_manager* manager,
                                                               const std::string& inputString)) {
        if (this->mpi_manager_) {
            this->mpi_manager_->set_serialize_function(serializeModule);
        }
    }
    void set_deserialize_function(void (*deserializeModule)(mpi_manager* manager,
                                                            const std::string& parameter,
                                                            const std::string& inputString)) {
        if (this->mpi_manager_) {
            this->mpi_manager_->set_deserialize_function(deserializeModule);
        }
    }
};

/**
 * @brief Main_process's job is to load, divide, and distribute modules from a config file.
 */
class main_process : public process {
  private:
    int process_count_ = 0;
    module_manager module_manager_;
    std::string conf_path_;
    bool (*divide_function_)(std::vector<module_info*>* modules, int nodeCount);
    void (*add_instruction_)(module_info* mod, std::string* instructions);

  public:
    // main_process class implementation
    main_process(int rank, int process_count) : process(rank), process_count_(process_count){};

    void set_conf_path(const std::string& path) { this->conf_path_ = path; };
    void set_divide_function(bool (*divide)(std::vector<module_info*>* modules, int nodeCount)) {
        this->divide_function_ = divide;
    };
    void set_add_instruction(void (*addInstruction)(module_info* mod, std::string* instructions)) {
        this->add_instruction_ = addInstruction;
    };
    void process_information() override;
};

/**
 * @brief Represents the rest of the processes that are not main.
 *
 * Their only job is to complete assigned instructions with assigned modules.
 */
class slave_process : public process {
  public:
    slave_process(int rank) : process(rank){};
    void process_information() override;
};
