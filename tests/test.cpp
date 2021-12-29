#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "Ipc.hpp"

using namespace std::chrono_literals;

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
    int pid;

    if ((pid = fork()) == -1) {
        perror("fork");
        ASSERT_THROW(false);
    }
    else if (pid > 0) {
        // Parent process
        Ipc::Server server;
        server.init("IpcTest");

        std::cout << "Server: Waiting for client to connect..." << std::endl;
        std::cout.flush();

        Ipc::Connection connection = server.accept();
        std::cout << "Server: Client connected" << std::endl;
        std::cout.flush();

        char buffer[BUF_SIZE];
        size_t bytesReceived = 0;
        size_t bytesAvailable = 0;
        bool success = connection.peek(buffer, BUF_SIZE, &bytesReceived, &bytesAvailable);
        ASSERT_THROW(success);

        std::cout
            << "Server: Peek'd " << bytesReceived
            << " bytes (out of " << bytesAvailable
            << " bytes available) from client: " << buffer << std::endl;
        std::cout.flush();

        ASSERT_THROW(strncmp(buffer, CLIENT_MESSAGE, bytesReceived) == 0);

        success = connection.recv(buffer, BUF_SIZE, &bytesReceived);
        ASSERT_THROW(success);

        std::cout
            << "Server: Received " << bytesReceived
            << " bytes from client: " << buffer << std::endl;
        std::cout.flush();

        ASSERT_THROW(bytesReceived == (strlen(CLIENT_MESSAGE) + 1));
        ASSERT_THROW(strncmp(buffer, CLIENT_MESSAGE, BUF_SIZE) == 0);

        size_t bytesSent = 0;
        success = connection.send(SERVER_MESSAGE, strlen(SERVER_MESSAGE) + 1, &bytesSent);
        ASSERT_THROW(success);

        std::cout
            << "Server: Sent " << bytesSent
            << " bytes to client: " << SERVER_MESSAGE << std::endl;

        ASSERT_THROW(bytesSent == (strlen(SERVER_MESSAGE) + 1));

        int waitedpid = wait(NULL);
        ASSERT_THROW(waitedpid == pid);
    }
    else {
        // Child process
        std::this_thread::sleep_for(100ms);

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
        ASSERT_THROW(strncmp(buffer, SERVER_MESSAGE, BUF_SIZE) == 0);
    }

    return 0;
}

#ifdef __cplusplus
};
#endif
