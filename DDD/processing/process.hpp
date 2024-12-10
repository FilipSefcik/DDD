#pragma once
#include "../utils/divider.hpp"
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
};

/**
 * @brief Main_process's job is to load, divide, and distribute modules from a config file.
 */
class main_process : public process {
  private:
    int process_count_ = 0;
    module_manager module_manager_;
    divider* divider_ = nullptr;
    std::string conf_path_;

  public:
    // main_process class implementation
    main_process(int rank, int process_count) : process(rank), process_count_(process_count){};
    ~main_process();

    void set_conf_path(std::string path) { this->conf_path_ = path; };
    void process_information() override;
    void set_divider(int flag);
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
