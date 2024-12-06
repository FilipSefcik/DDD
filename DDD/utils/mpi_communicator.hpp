#pragma once
#include <mpi.h>
#include <string>
#include <vector>

/**
 * @brief mpi_communicator simplifies usage of MPI message functions.
 * Now unified around the Message struct for communication.
 */
class mpi_communicator {
  public:
    /**
     * @brief Message struct encapsulates header and payload.
     */
    struct mpi_message {
        std::string header_;
        std::string payload_;
        std::string delimiter_ = "\n#\n";
    };

    static void send_message(const mpi_message& message, int receiverRank);
    static void recv_message(int senderRank, mpi_message& message);

    static void scatter_ints(int* message, int* receiveBuffer);
    static void scatter_messages(const std::vector<mpi_message>& messages,
                                 mpi_message& receiveBuffer);
    static void gather_doubles(double* message, double* receiveBuffer);

  private:
    static void serialize_message(const mpi_message message, std::string& serializedMessage);
    static void deserialize_message(const std::string& serializedMessage, mpi_message& message);
};
