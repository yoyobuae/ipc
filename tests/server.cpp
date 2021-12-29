#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

#include "Ipc.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#define ASSERT_THROW(condition)                                     \
{                                                                   \
  if( !( condition ) )                                              \
  {                                                                 \
    throw std::runtime_error(   std::string( __FILE__ )             \
                              + std::string( ":" )                  \
                              + std::to_string( __LINE__ )          \
                              + std::string( " in " )               \
                              + std::string( __PRETTY_FUNCTION__ )  \
                              + std::string( ": Assert failed: " )  \
                              + std::string( #condition )           \
    );                                                              \
  }                                                                 \
}

#define CLIENT_MESSAGE "Hello server"
#define SERVER_MESSAGE "Hi client"
#define BUF_SIZE 20

int main(int, char **)
{
    Ipc::Server server;
    server.init("IpcTest");

    std::cout << "Server: Waiting for client to connect..." << std::endl;
    std::cout.flush();

    Ipc::Connection connection = server.accept();
    std::cout << "Server: Client connected" << std::endl;
    std::cout.flush();

    std::cout
        << "Server: Receiving" << std::endl;
    std::cout.flush();

    char buffer[BUF_SIZE];
    size_t bytesReceived = 0;
    bool success = connection.recv(buffer, BUF_SIZE, &bytesReceived);
    ASSERT_THROW(success);

    std::cout
        << "Server: Received " << bytesReceived
        << " bytes from client: " << buffer << std::endl;
    std::cout.flush();

    ASSERT_THROW(bytesReceived == (strlen(CLIENT_MESSAGE) + 1));
    ASSERT_THROW(strncmp(buffer, CLIENT_MESSAGE, BUF_SIZE) == 0);

    std::cout
        << "Server: Sending " << (strlen(SERVER_MESSAGE) + 1)
        << " bytes to client: " << SERVER_MESSAGE << std::endl;
    std::cout.flush();

    size_t bytesSent = 0;
    success = connection.send(SERVER_MESSAGE, strlen(SERVER_MESSAGE) + 1, &bytesSent);
    ASSERT_THROW(success);

    std::cout
        << "Server: Sent " << bytesSent << " bytes" << std::endl;

    ASSERT_THROW(bytesSent == (strlen(SERVER_MESSAGE) + 1));

    return 0;
}

#ifdef __cplusplus
};
#endif
