#pragma once
#include <mpi.h>
#include <string>

/*
 * @brief mpi_communicator simplifies usage of MPI message functions.
 * Now unified around the Message struct for communication.
 */
class mpi_communicator {
  public:
    /*
     * @brief Message struct encapsulates header and payload.
     */
    struct mpi_message {
        std::string header_;
        std::string payload_;
    };

    static void send_message(const mpi_message& message, int receiverRank);
    static void recv_message(int senderRank, mpi_message& message);

    static void scatter_ints(int* message, int* receiveBuffer);
    static void gather_doubles(double* message, double* receiveBuffer);
};
