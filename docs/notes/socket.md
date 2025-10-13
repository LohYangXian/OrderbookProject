# Socket Programming in C++

Socket programming is a way to enable communication between two nodes on a network. In C++, socket programming can be done using the Berkeley sockets API, which provides a set of functions for creating and managing sockets. Below are the basic steps to create a simple TCP server and client using sockets in C++.

## Creating Client and Server Sockets

```cpp

socket(AF_INET, SOCK_STREAM, 0);
```

- `AF_INET`: Specifies the address family (IPv4).
- `SOCK_STREAM`: Specifies the socket type (TCP).
- `0`: Specifies the protocol (default for TCP).

Other common socket types include:
- `SOCK_DGRAM`: For UDP (datagram) sockets.
- `AF_INET6`: For IPv6 addresses.


## Basic Steps to Create a TCP Server
1. **Create a socket** using the `socket()` function.
2. **Bind the socket** to an IP address and port using the `bind()` function.
3. **Listen for incoming connections** using the `listen()` function.
4. **Accept a connection** using the `accept()` function.
5. **Receive and send data** using the `recv()` and `send()` functions.

