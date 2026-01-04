#pragma once

#include <string>
#include <cstdint>

namespace TopSpeed {

/**
 * UDPSocket provides low-latency, connectionless communication
 * Suitable for real-time game data transmission
 */
class UDPSocket {
public:
    UDPSocket();
    ~UDPSocket();

    /**
     * Create a UDP socket
     * @return true if successful
     */
    bool Create();

    /**
     * Bind socket to a local port (for server/listener)
     * @param port Local port to bind to
     * @return true if successful
     */
    bool Bind(uint16_t port);

    /**
     * Send data to specific address
     * @param address IP address or hostname
     * @param port Destination port
     * @param data Pointer to data
     * @param size Size of data
     * @return Number of bytes sent, or -1 on error
     */
    int SendTo(const std::string& address, uint16_t port, const char* data, size_t size);

    /**
     * Receive data from any source
     * @param buffer Buffer to receive into
     * @param buffer_size Size of buffer
     * @param from_address Output: address of sender
     * @param from_port Output: port of sender
     * @return Number of bytes received, or -1 on error
     */
    int ReceiveFrom(char* buffer, size_t buffer_size, std::string& from_address, uint16_t& from_port);

    /**
     * Close the socket
     */
    void Close();

    /**
     * Check if socket is open
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
     * Get last error message
     * @return Error description
     */
    const std::string& GetLastError() const { return last_error_; }

private:
    static constexpr uintptr_t INVALID_SOCKET_VALUE = ~0ULL;
    uintptr_t socket_handle_;
    std::string last_error_;

    void SetError(const std::string& error);
};

} // namespace TopSpeed
