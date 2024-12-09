#include "mpi_communicator.hpp"
#include <cstring>
#include <iostream>
#include <mpi.h>

void mpi_communicator::send_message(const mpi_message& message, int receiverRank) {
    std::string msgBundle;
    serialize_message(message, msgBundle);

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

    deserialize_message(msgBundle, message);
}

void mpi_communicator::scatter_ints(int* message, int* receiveBuffer) {
    MPI_Scatter(message, 1, MPI_INT, receiveBuffer, 1, MPI_INT, 0, MPI_COMM_WORLD);
}

void mpi_communicator::scatter_messages(const std::vector<mpi_message>* messages,
                                        mpi_message& receiveBuffer) {
    int mpiSize;
    int rank;
    MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int maxMessageSize = 0;

    // Only the root process serializes and calculates message sizes
    std::vector<std::string> serializedMessages;
    if (rank == 0) {
        for (const auto& message : *messages) {
            std::string serializedMessage;
            serialize_message(message, serializedMessage);
            serializedMessages.push_back(serializedMessage);
            maxMessageSize =
                std::max(maxMessageSize, static_cast<int>(serializedMessage.size() + 1));
        }
    }

    // Broadcast the maximum message size
    MPI_Bcast(&maxMessageSize, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Root process prepares the send buffer
    char* sendBuffer = nullptr;
    if (rank == 0) {
        sendBuffer = new char[mpiSize * maxMessageSize];
        for (size_t i = 0; i < serializedMessages.size(); ++i) {
            std::strncpy(&sendBuffer[i * maxMessageSize], serializedMessages[i].c_str(),
                         maxMessageSize);
        }
    }

    // Allocate buffer for receiving one message
    char* receiveBufferRaw = new char[maxMessageSize];

    // Scatter the serialized messages
    MPI_Scatter(sendBuffer, maxMessageSize, MPI_CHAR, receiveBufferRaw, maxMessageSize, MPI_CHAR, 0,
                MPI_COMM_WORLD);

    // Deserialize the received message
    std::string receivedString(receiveBufferRaw);
    deserialize_message(receivedString, receiveBuffer);

    // Clean up
    delete[] receiveBufferRaw;
    if (rank == 0) {
        delete[] sendBuffer;
    }
}

void mpi_communicator::gather_doubles(double* message, double* receiveBuffer) {
    MPI_Gather(message, 1, MPI_DOUBLE, receiveBuffer, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

void mpi_communicator::print_message(mpi_message message) {
    std::cout << "Header: " << message.header_ << std::endl;
    std::cout << "Payload: " << message.payload_ << std::endl;
}

void mpi_communicator::serialize_message(const mpi_message message,
                                         std::string& serializedMessage) {
    serializedMessage = message.header_ + message.delimiter_ + message.payload_;
}

void mpi_communicator::deserialize_message(const std::string& serializedMessage,
                                           mpi_message& message) {
    size_t delimiterPos = serializedMessage.find(message.delimiter_);
    message.header_ = serializedMessage.substr(0, delimiterPos);
    message.payload_ = serializedMessage.substr(delimiterPos + 3);
}