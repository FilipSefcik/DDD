#include "ddd.hpp"
#include "../utils/callbacks.hpp"
#include "../utils/mpi_communicator.hpp"
#include <iostream>

ddd::ddd() {
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

    // Get the number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &this->process_count);

    // Get the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &this->my_rank);

    if (this->my_rank == 0) {
        this->process_ = new main_process(this->my_rank, this->process_count);
    } else {
        this->process_ = new slave_process(this->my_rank);
    }

    // char processorName[MPI_MAX_PROCESSOR_NAME];
    // int nameLength;
    // MPI_Get_processor_name(processorName, &nameLength); // Get the processor name
    // std::cout << "Rank " << this->my_rank << " runs on " << processorName << std::endl;
}

ddd::~ddd() {
    delete this->process_;
    // Finalize the MPI environment.
    MPI_Finalize();
}

void ddd::set_conf_path(const std::string& pa_conf_path) {
    if (this->my_rank == 0 && this->process_) {
        main_process* mainProcess = dynamic_cast<main_process*>(this->process_);
        mainProcess->set_conf_path(pa_conf_path);
    }
}

void ddd::calculate_availability(int divider_flag, int state, bool timer_on) {
    if (timer_on) {
        this->start_time = MPI_Wtime();
    }

    if (this->my_rank == 0 && this->process_) {
        main_process* mainProcess = dynamic_cast<main_process*>(this->process_);
        switch (divider_flag) {
            case 0:
                mainProcess->set_divide_function(divide_by_var_count);
                break;
            case 1:
                mainProcess->set_divide_function(divide_for_merging);
                break;
            default:
                mainProcess->set_divide_function(divide_evenly);
                break;
        }
        // mainProcess->set_add_instruction(add_instruction_density);
        mainProcess->set_add_instruction(add_instruction_merging);
    }
    this->process_->process_information();
    this->process_->set_function(execute_merging);
    this->process_->set_serialize_function(serialize_merging);
    this->process_->set_deserialize_function(deserialize_merging);
    // this->process_->set_function(calculate_true_density);
    // this->process_->set_serialize_function(serialize_true_density);
    // this->process_->set_deserialize_function(deserialize_true_density);
    this->process_->process_instructions(state);

    if (timer_on) {
        this->end_time = MPI_Wtime();
    }
}

void ddd::get_max_time() {
    double min_start, max_end;

    std::vector<double> start_times, end_times;
    start_times.resize(this->process_count);
    end_times.resize(this->process_count);

    mpi_communicator::gather_doubles(&this->start_time, start_times.data());
    mpi_communicator::gather_doubles(&this->end_time, end_times.data());

    if (this->my_rank == 0) {
        min_start = std::numeric_limits<double>::max();
        max_end = std::numeric_limits<double>::min();
        for (int i = 0; i < this->process_count; i++) {
            min_start = min_start > start_times.at(i) ? start_times.at(i) : min_start;
            max_end = max_end < end_times.at(i) ? end_times.at(i) : max_end;
        }

        std::cout << "Time: " << max_end - min_start << std::endl;
    }
}
