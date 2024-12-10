#include "process.hpp"

// -------process---------
void process::process_instructions(int state) {
    if (this->mpi_manager_) {
        this->mpi_manager_->complete_instructions(this->my_instructions_, state);
    }
}

void process::deserialize_message(mpi_communicator::mpi_message message) {
    if (message.header_ == "MODULE") {
        size_t delimiterPos = message.payload_.find(message.delimiter_);
        this->my_instructions_ = message.payload_.substr(0, delimiterPos);
        std::string modulesInfo = message.payload_.substr(delimiterPos + 3);
        this->mpi_manager_ = new mpi_manager(modulesInfo);
    }
}

process::~process() {
    if (mpi_manager_) {
        delete mpi_manager_;
        mpi_manager_ = nullptr;
    }
}


// -------main process-------

main_process::~main_process() {
    if (divider_) {
        delete divider_;
        divider_ = nullptr; // Avoid dangling pointer
    }
}

void main_process::process_information() {
    this->module_manager_.load(this->conf_path_);
    this->divider_->divide_modules(this->module_manager_.get_modules(), this->process_count_);

    this->module_manager_.get_instructions(this->process_count_);

    std::vector<mpi_communicator::mpi_message> messages;
    this->module_manager_.create_messages(this->process_count_, messages);

    mpi_communicator::mpi_message myMessage;
    mpi_communicator::scatter_messages(&messages, myMessage);
    this->deserialize_message(myMessage);
}

void main_process::set_divider(int flag) {
    switch (flag) {
        case 0:
            this->divider_ = new var_count_divider();
            break;
        default:
            this->divider_ = new node_divider();
            break;
    }
}

// ------slave process-------

void slave_process::process_information() {
    mpi_communicator::mpi_message myMessage;
    mpi_communicator::scatter_messages(nullptr, myMessage);
    this->deserialize_message(myMessage);
}
