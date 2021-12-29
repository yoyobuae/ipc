# IPC Library

Inter-process communication library.

Simple library meant to be a drop-in replacement for Windows NamedPipes
providing cross-platform compatibility between Linux and Windows.

## Compiling

- `git clone https://github.com/yoyobuae/ipc.git`
- `cd ipc`
- `cmake -B build`
- `cmake --build build`

After compiling run test program with:
- `./build/tests/ipc_test`

## Usage

Create server, wait for connection, receive message from client and echo
the same message back to client:

```cpp
bool success = false;
char buffer[20];
size_t bytesReceived;
size_t bytesSent;
Ipc::Server server;

server.init("Example");

Ipc::Connection connection = server.accept();

if (connection.isInvalid() == false) {
    success = connection.recv(buffer, 20, &bytesReceived);
}

if (success) {
    success = connection.send(buffer, bytesReceived, &bytesSent)
}
```

Create client, connect to server, send message and receive server response:
```cpp
bool success = false;
char buffer[20];
size_t bytesReceived;
size_t bytesSent;

Ipc::Client client("Example");

Ipc::Connection connection = client.connect();

if (connection.isInvalid() == false) {
    success = connection.send("foobar", strlen("foobar") + 1, &bytesSent);
}

if (success) {
    success = connection.recv(buffer, 20, &bytesReceived);
}
```

Check test programs for more examples of usage.

## License

All files in this repo are covered under MIT License:

```
MIT License

Copyright (c) 2021 Gerald Young (Yoyobuae)

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the “Software”),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
```
