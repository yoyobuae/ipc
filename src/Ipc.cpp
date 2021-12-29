/*
 * MIT License
 *
 * Copyright (c) 2021 Gerald Young (Yoyobuae)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the “Software”),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <sstream>

#if defined(__linux) || defined(__linux__) || defined(linux)
#include <errno.h>
#include <linux/sockios.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#endif

#include "Ipc.hpp"

namespace Ipc {

    const int BUFSIZE = 1024;

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    Connection::Connection(HANDLE inPipe) : inPipe(inPipe) { }

    Connection::~Connection()
    {
        if (!isInvalid())
            DisconnectNamedPipe(inPipe);
    }

    bool Connection::send(const char *src, size_t srcSize, size_t *bytesSent)
    {
        if (isInvalid()) return false;

        DWORD dwWritten;
        if (WriteFile(inPipe, src, srcSize, &dwWritten, NULL) != FALSE)
        {
            if (bytesSent) *bytesSent = dwWritten;
            return true;
        }
        else
        {
            return false
        }
    }

    bool Connection::recv(char *dst, size_t dstSize, size_t *bytesReceived)
    {
        if (isInvalid()) return false;

        DWORD dwRead;
        if (ReadFile(inPipe, dst, dstSize, &dwRead, NULL) != FALSE)
        {
            if (bytesReceived) *bytesReceived = dwRead;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool Connection::peek(char *dst, size_t dstSize,
                          size_t *bytesReceived, size_t *bytesAvailable)
    {
        if (isInvalid()) return false;

        DWORD dwRead;
        DWORD dwAvailable;
        if (PeekNamedPipe(inPipe, dst, dstSize, &dwRead, &dwAvailable, NULL)) {
            if (bytesReceived) *bytesReceived = dwRead;
            if (bytesAvailable) *bytesAvailable = dwAvailable;
            return true;
        }
        else {
            return false;
        }
    }

    bool Connection::isInvalid()
    {
        return inPipe == INVALID_HANDLE_VALUE;
    }

    Server::Server() { }

    void Server::init(std::string name)
    {
        std::stringstream ss;
        ss << "\\\\.\\pipe\\" << name;
        std::string inPipeName = ss.str();

        inPipe = CreateNamedPipeA(inPipeName.c_str(),
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE |PIPE_WAIT,   // FILE_FLAG_FIRST_PIPE_INSTANCE is not needed but forces CreateNamedPipe(..) to fail if the pipe already exists...
            1,
            1024 * 16,
            1024 * 16,
            NMPWAIT_USE_DEFAULT_WAIT,
            NULL);
    }

    Connection Server::accept()
    {
        ConnectNamedPipe(inPipe, NULL);
        return Connection(inPipe);
    }

    Client::Client(std::string name) : name(name) { }

    bool Client::sendrecv(char *dst, size_t dstSize, const char *src, size_t srcSize,
                          size_t *bytesReceived = NULL)
    {
        CHAR chReadBuf[BUFSIZE];
        BOOL fSuccess;
        DWORD cbRead;

        std::stringstream ss;
        ss << "\\\\.\\pipe\\" << name;
        std::string outPipeName = ss.str();

        fSuccess = CallNamedPipeA(
            outPipeName.c_str(),        // pipe name 
            src,           // message to server 
            srcSize, // message length 
            dst,              // buffer to receive reply 
            dstSize,  // size of read buffer 
            &cbRead,                // number of bytes read 
            2000);                 // waits for 2 seconds 

        if (fSuccess || GetLastError() == ERROR_MORE_DATA)
        {
            if (bytesReceived) *bytesReceived = cbRead;
            return true;
        }
        else
        {
            return false;
        }
    }

    Connection Client::connect()
    {
        std::stringstream ss;
        ss << "\\\\.\\pipe\\" << name;
        std::string inPipeName = ss.str();

        HANDLE inPipe = CreateFileA(inPipeName.c_str(),
                                    GENERIC_READ | GENERIC_WRITE,
                                    0,
                                    NULL,
                                    OPEN_EXISTING,
                                    0, // TODO : Overlapped
                                    NULL);
        return Connection(inPipe);
    }

#elif defined(__linux) || defined(__linux__) || defined(linux)

    Server::Server() { }

    void Server::init(std::string name)
    {
        struct sockaddr_un local;

        if ((listenfd = ::socket(AF_UNIX, SOCK_SEQPACKET, 0)) == -1) {
            perror("socket");
        }

        std::stringstream ss;
        ss << "/tmp/" << name;
        std::string sockpath = ss.str();

        local.sun_family = AF_UNIX;
        strcpy(local.sun_path, sockpath.c_str());
        unlink(local.sun_path);
        int len = strlen(local.sun_path) + sizeof(local.sun_family);
        if (::bind(listenfd, (struct sockaddr *)&local, len) == -1) {
            perror("bind");
        }

        if (::listen(listenfd, 5) == -1) {
            perror("listen");
        }
    }

    Connection Server::accept()
    {
        struct sockaddr_un remote;
        int connfd;
        socklen_t t = sizeof(remote);
        if ((connfd = ::accept(listenfd, (struct sockaddr *)&remote, &t)) == -1) {
            perror("accept");
        }
        return Connection(connfd);
    }

    Connection::Connection(int connfd) : connfd(connfd) { }

    Connection::~Connection()
    {
        if (!isInvalid())
            ::close(connfd);
    }
    bool Connection::send(const char *src, size_t srcSize, size_t *bytesSent)
    {
        if (isInvalid()) return false;

        bool ret = true;
        ssize_t sent = 0;
        if ((sent = ::send(connfd, src, srcSize, 0)) < 0) {
            perror("send");
            ret = false;
        }
        else {
            if (bytesSent) *bytesSent = sent;
        }

        return ret;
    }

    bool Connection::recv(char *dst, size_t dstSize, size_t *bytesReceived)
    {
        if (isInvalid()) return false;

        bool ret = true;
        ssize_t received;
        received = ::recv(connfd, dst, dstSize, 0);
        if (received < 0) {
            perror("recv");
            ret = false;
        }
        else {
            if (bytesReceived) *bytesReceived = received;
        }
        return ret;
    }

    bool Connection::peek(char *dst, size_t dstSize,
                          size_t *bytesReceived, size_t *bytesAvailable)
    {
        if (isInvalid()) return false;

        bool ret = true;
        ssize_t received;
        received = ::recv(connfd, dst, dstSize, MSG_PEEK);
        if (received < 0) {
            perror("recv");
            ret = false;
        }
        else {
            if (bytesReceived) *bytesReceived = received;
            int available = 0;
            if (::ioctl(connfd, FIONREAD, &available) >= 0) *bytesAvailable = available;
        }
        return ret;
    }

    bool Connection::isInvalid()
    {
        return connfd < 0;
    }

    Client::Client(std::string name) : name(name) { }

    bool Client::sendrecv(char *dst, size_t dstSize, const char *src, size_t srcSize,
                          size_t *bytesReceived)
    {
        Connection connection = connect();
        if (connection.isInvalid()) return false;

        bool success = connection.send(src, srcSize, NULL);
        if (!success) return false;

        success = connection.recv(dst, dstSize, bytesReceived);
        if (!success) return false;

        return true;
    }

    Connection Client::connect()
    {
        int s, t, len;
        struct sockaddr_un remote;

        if ((s = ::socket(AF_UNIX, SOCK_SEQPACKET, 0)) == -1) {
            perror("socket");
            return Connection(-1);
        }

        std::stringstream ss;
        ss << "/tmp/" << name;
        std::string sockpath = ss.str();

        remote.sun_family = AF_UNIX;
        strcpy(remote.sun_path, sockpath.c_str());
        len = strlen(remote.sun_path) + sizeof(remote.sun_family);
        if (::connect(s, (struct sockaddr *)&remote, len) == -1) {
            perror("connect");
            close(s);
            return Connection(-1);
        }
        return Connection(s);
    }

#else

    Server::Server() { }
    void Server::init(std::string name) { }
    Connection Server::accept()
    {
        return Connection();
    }

    Connection::Connection() { }
    Connection::~Connection() { }
    bool Connection::send(const char *src, size_t srcSize, size_t *bytesSent)
    {
        return false;
    }
    bool Connection::recv(char *dst, size_t dstSize, size_t *bytesReceived)
    {
        return false;
    }
    bool Connection::peek(char *dst, size_t dstSize,
                          size_t *bytesReceived, size_t *bytesAvailable)
    {
        return false;
    }
    bool Connection::isInvalid()
    {
        return true;
    }

    Client::Client(std::string name) : name(name) { }

    bool Client::sendrecv(char *dst, size_t dstSize, const char *src, size_t srcSize,
                          size_t *bytesReceived)
    {
        return false;
    }

    Connection Client::connect()
    {
        return Connection();
    }

#endif

}; // namespace Ipc
