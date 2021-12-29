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
    Ipc::Client client("IpcTest");

    std::cout
        << "Client: Sending " << (strlen(CLIENT_MESSAGE) + 1)
        << " bytes to server: " << CLIENT_MESSAGE << std::endl;
    std::cout.flush();

    char buffer[BUF_SIZE];
    size_t bytesReceived = 0;
    bool success = client.sendrecv(buffer, BUF_SIZE, CLIENT_MESSAGE, strlen(CLIENT_MESSAGE) + 1, &bytesReceived);
    ASSERT_THROW(success);

    std::cout
        << "Client: Received " << bytesReceived
        << " bytes from server: " << buffer << std::endl;
    std::cout.flush();

    ASSERT_THROW(bytesReceived == (strlen(SERVER_MESSAGE) + 1));
    ASSERT_THROW(strncmp(buffer, SERVER_MESSAGE, BUF_SIZE));

    return 0;
}

#ifdef __cplusplus
};
#endif
