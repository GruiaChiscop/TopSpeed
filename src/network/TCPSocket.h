#pragma once

#include <string>
#include <cstdint>

namespace TopSpeed {

/**
 * TCPSocket provides reliable, ordered data transmission
 * Suitable for game state synchronization and critical data
 */
class TCPSocket {
public:
    TCPSocket();
    ~TCPSocket();

    /**
     * Create a TCP socket
     * @return true if successful
     */
    bool Create();

    /**
     * Connect to a remote server
     * @param address IP address or hostname
     * @param port Remote port
     * @return true if successful
     */
    bool Connect(const std::string& address, uint16_t port);

    /**
     * Bind socket to a local port (for server)
     * @param port Local port to bind to
     * @return true if successful
     */
    bool Bind(uint16_t port);

    /**
     * Listen for incoming connections (for server)
     * @param backlog Maximum pending connections
     * @return true if successful
     */
    bool Listen(int backlog = 5);

    /**
     * Accept incoming connection
     * @return New socket for accepted connection, or nullptr on error
     */
    TCPSocket* Accept();

    /**
     * Send data to connected peer
     * @param data Pointer to data
     * @param size Size of data
     * @return Number of bytes sent, or -1 on error
     */
    int Send(const char* data, size_t size);

    /**
     * Receive data from connected peer
     * @param buffer Buffer to receive into
     * @param buffer_size Size of buffer
     * @return Number of bytes received, 0 if connection closed, -1 on error
     */
    int Receive(char* buffer, size_t buffer_size);

    /**
     * Close the socket
     */
    void Close();

    /**
     * Check if socket is connected/open
     * @return true if open
     */
    bool IsOpen() const { return socket_handle_ != INVALID_SOCKET_VALUE; }

    /**
     * Set socket to non-blocking mode
     * @param non_blocking true for non-blocking
     * @return true if successful
     */
    bool SetNonBlocking(bool non_blocking);

    /**
     * Get remote peer address
     * @return Address string
     */
    const std::string& GetRemoteAddress() const { return remote_address_; }

    /**
     * Get remote peer port
     * @return Port number
     */
    uint16_t GetRemotePort() const { return remote_port_; }

    /**
     * Get last error message
     * @return Error description
     */
    const std::string& GetLastError() const { return last_error_; }

private:
    static constexpr uintptr_t INVALID_SOCKET_VALUE = ~0ULL;
    uintptr_t socket_handle_;
    std::string remote_address_;
    uint16_t remote_port_;
    std::string last_error_;

    // Private constructor for Accept()
    TCPSocket(uintptr_t socket, const std::string& remote_addr, uint16_t remote_port);

    void SetError(const std::string& error);

    friend class TCPSocket;  // For Accept() to create new sockets
};

} // namespace TopSpeed
