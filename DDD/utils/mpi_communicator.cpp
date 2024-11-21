#include "mpi_communicator.hpp"

void mpi_communicator::send_message(const mpi_message& message, int receiverRank) {
    std::string msgBundle = message.header_ + "#\n#" + message.payload_;

    int msgSize = msgBundle.size() + 1;
    MPI_Send(&msgSize, 1, MPI_INT, receiverRank, 0, MPI_COMM_WORLD);

    MPI_Send(msgBundle.c_str(), msgSize, MPI_CHAR, receiverRank, 0, MPI_COMM_WORLD);
}

void mpi_communicator::recv_message(int senderRank, mpi_message& message) {
    int msgSize;
    MPI_Recv(&msgSize, 1, MPI_INT, senderRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    char* buffer = new char[msgSize];
    MPI_Recv(buffer, msgSize, MPI_CHAR, senderRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    std::string msgBundle(buffer);
    delete[] buffer;

    size_t delimiter = msgBundle.find("#\n#");
    message.header_ = msgBundle.substr(0, delimiter);
    message.payload_ = msgBundle.substr(delimiter + 3);
}

void mpi_communicator::scatter_ints(int* message, int* receiveBuffer) {
    MPI_Scatter(message, 1, MPI_INT, receiveBuffer, 1, MPI_INT, 0, MPI_COMM_WORLD);
}

void mpi_communicator::gather_doubles(double* message, double* receiveBuffer) {
    MPI_Gather(message, 1, MPI_DOUBLE, receiveBuffer, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}
