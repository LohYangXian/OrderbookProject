# Notes

## WebSocket++ TLS vs Non-TLS Client Notes

### What is TLS?

- **TLS (Transport Layer Security)** is a protocol for encrypting data sent over a network.
- It ensures privacy and integrity between client and server.
- `wss://` URLs use TLS (secure WebSocket).
- `ws://` URLs do not use TLS (plain, unencrypted WebSocket).

### WebSocket++ Client Types

- **Non-TLS Client:**  
  - Uses `websocketpp::config::asio_no_tls_client`
  - For `ws://` endpoints (no encryption)
  - Example:
    ```cpp
    #include <websocketpp/config/asio_no_tls_client.hpp>
    typedef websocketpp::client<websocketpp::config::asio_no_tls_client> client;
    ```

- **TLS Client:**  
  - Uses `websocketpp::config::asio_client` or `asio_tls_client`
  - For `wss://` endpoints (encrypted)
  - Requires OpenSSL
  - Example:
    ```cpp
    #include <websocketpp/config/asio_client.hpp>
    typedef websocketpp::client<websocketpp::config::asio_client> client;
    ```

### When to Use Each

- Use **TLS client** for any `wss://` connection (e.g., Binance, most exchanges).
- Use **non-TLS client** only for local or test servers with `ws://`.

### Why Does This Matter?

- Using the wrong client type will cause connection failures.
- TLS client requires linking OpenSSL and handling certificates.
- OpenSSL is an open-source library that provides cryptographic functions and secure communication (SSL/TLS) for applications.
It is widely used to enable encrypted network connections, such as HTTPS and secure WebSocket (wss://) connections.

---