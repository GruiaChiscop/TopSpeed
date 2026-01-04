#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <functional>

namespace TopSpeed {

/**
 * NetworkManager provides cross-platform networking using modern UDP/TCP
 * Replaces deprecated DirectPlay with modern socket-based networking
 */
class NetworkManager {
public:
    enum class ProtocolType {
        UDP,  // For real-time game data
        TCP   // For reliable game state
    };

    using MessageCallback = std::function<void(const std::string&, size_t)>;

    NetworkManager();
    ~NetworkManager();

    /**
     * Initialize networking subsystem
     * @return true if successful
     */
    bool Initialize();

    /**
     * Connect to a server
     * @param host Hostname or IP address
     * @param port Port number
     * @param protocol Protocol type (UDP or TCP)
     * @return true if connection successful
     */
    bool Connect(const std::string& host, uint16_t port, ProtocolType protocol = ProtocolType::UDP);

    /**
     * Disconnect from server
     */
    void Disconnect();

    /**
     * Start listening for incoming connections
     * @param port Port to listen on
     * @param protocol Protocol type
     * @return true if successful
     */
    bool Listen(uint16_t port, ProtocolType protocol = ProtocolType::TCP);

    /**
     * Send data to connected peer
     * @param data Data to send
     * @param size Size of data
     * @return true if sent successfully
     */
    bool Send(const char* data, size_t size);

    /**
     * Send data to specific address
     * @param host Target hostname or IP
     * @param port Target port
     * @param data Data to send
     * @param size Size of data
     * @return true if sent successfully
     */
    bool SendTo(const std::string& host, uint16_t port, const char* data, size_t size);

    /**
     * Receive data from peer
     * @param buffer Buffer to receive into
     * @param buffer_size Size of buffer
     * @return Number of bytes received
     */
    size_t Receive(char* buffer, size_t buffer_size);

    /**
     * Set callback for incoming messages
     * @param callback Function to call when data received
     */
    void SetMessageCallback(MessageCallback callback);

    /**
     * Check if connected
     * @return true if actively connected
     */
    bool IsConnected() const { return connected_; }

    /**
     * Update network (process pending messages)
     */
    void Update();

    /**
     * Shutdown networking
     */
    void Shutdown();

private:
    bool connected_;
    ProtocolType protocol_type_;
    MessageCallback message_callback_;
    // TODO: Add actual socket handles/pointers
    // SOCKET socket_;
    // Or use Boost.Asio:
    // std::unique_ptr<boost::asio::io_context> io_context_;
    // std::unique_ptr<boost::asio::ip::udp::socket> socket_;
};

} // namespace TopSpeed
