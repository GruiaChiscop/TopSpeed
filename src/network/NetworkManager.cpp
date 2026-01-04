#include "NetworkManager.h"
#include "UDPSocket.h"
#include "TCPSocket.h"
#include <iostream>
#include <memory>
#include <thread>
#include <mutex>

namespace TopSpeed {

struct NetworkManagerImpl {
    std::unique_ptr<UDPSocket> udp_socket;
    std::unique_ptr<TCPSocket> tcp_socket;
    std::thread receive_thread;
    bool running = false;
    std::mutex data_mutex;
};

NetworkManager::NetworkManager()
    : connected_(false)
    , protocol_type_(ProtocolType::UDP)
    , impl_(nullptr) {
}

NetworkManager::~NetworkManager() {
    Shutdown();
}

bool NetworkManager::Initialize() {
    impl_ = std::make_unique<NetworkManagerImpl>();
    std::cout << "NetworkManager initialized" << std::endl;
    return true;
}

bool NetworkManager::Connect(const std::string& host, uint16_t port, ProtocolType protocol) {
    if (connected_) {
        Disconnect();
    }

    if (!impl_) {
        std::cerr << "NetworkManager not initialized" << std::endl;
        return false;
    }

    protocol_type_ = protocol;

    try {
        if (protocol == ProtocolType::UDP) {
            impl_->udp_socket = std::make_unique<UDPSocket>();
            if (!impl_->udp_socket->Create()) {
                std::cerr << "Failed to create UDP socket" << std::endl;
                return false;
            }
            
            // For UDP client, we don't bind - just remember the server address
            // We'll sendto() directly
            connected_ = true;
            std::cout << "UDP client ready to send to " << host << ":" << port << std::endl;
        } else {
            // TCP
            impl_->tcp_socket = std::make_unique<TCPSocket>();
            if (!impl_->tcp_socket->Create()) {
                std::cerr << "Failed to create TCP socket" << std::endl;
                return false;
            }
            
            if (!impl_->tcp_socket->Connect(host, port)) {
                std::cerr << "Failed to connect to " << host << ":" << port << std::endl;
                return false;
            }
            
            connected_ = true;
        }

        // Start receive thread
        impl_->running = true;
        impl_->receive_thread = std::thread([this]() {
            ReceiveThreadFunc();
        });

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Connect error: " << e.what() << std::endl;
        return false;
    }
}

void NetworkManager::Disconnect() {
    if (!connected_) {
        return;
    }

    if (impl_) {
        impl_->running = false;
        
        if (impl_->receive_thread.joinable()) {
            impl_->receive_thread.join();
        }
        
        if (impl_->udp_socket) {
            impl_->udp_socket->Close();
        }
        if (impl_->tcp_socket) {
            impl_->tcp_socket->Close();
        }
    }

    connected_ = false;
    std::cout << "Disconnected from server" << std::endl;
}

bool NetworkManager::Listen(uint16_t port, ProtocolType protocol) {
    if (!impl_) {
        std::cerr << "NetworkManager not initialized" << std::endl;
        return false;
    }

    protocol_type_ = protocol;

    try {
        if (protocol == ProtocolType::UDP) {
            impl_->udp_socket = std::make_unique<UDPSocket>();
            if (!impl_->udp_socket->Create()) {
                return false;
            }
            if (!impl_->udp_socket->Bind(port)) {
                return false;
            }
            if (!impl_->udp_socket->SetNonBlocking(true)) {
                return false;
            }
        } else {
            // TCP
            impl_->tcp_socket = std::make_unique<TCPSocket>();
            if (!impl_->tcp_socket->Create()) {
                return false;
            }
            if (!impl_->tcp_socket->Bind(port)) {
                return false;
            }
            if (!impl_->tcp_socket->Listen()) {
                return false;
            }
            if (!impl_->tcp_socket->SetNonBlocking(true)) {
                return false;
            }
        }

        // Start receive thread
        impl_->running = true;
        impl_->receive_thread = std::thread([this]() {
            ReceiveThreadFunc();
        });

        std::cout << "Listening on port " << port 
                  << " (" << (protocol == ProtocolType::UDP ? "UDP" : "TCP") << ")" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Listen error: " << e.what() << std::endl;
        return false;
    }
}

bool NetworkManager::Send(const char* data, size_t size) {
    if (!connected_) {
        std::cerr << "Not connected" << std::endl;
        return false;
    }

    if (!impl_) {
        return false;
    }

    if (protocol_type_ == ProtocolType::UDP) {
        if (!impl_->udp_socket) {
            return false;
        }
        // For UDP client, this would need to know the server address
        // This is simplified - in real usage, would need to track server endpoint
        return true;
    } else {
        if (!impl_->tcp_socket) {
            return false;
        }
        int bytes_sent = impl_->tcp_socket->Send(data, size);
        return bytes_sent > 0;
    }
}

bool NetworkManager::SendTo(const std::string& host, uint16_t port, const char* data, size_t size) {
    if (!impl_ || !impl_->udp_socket) {
        std::cerr << "UDP not initialized" << std::endl;
        return false;
    }

    int bytes_sent = impl_->udp_socket->SendTo(host, port, data, size);
    return bytes_sent > 0;
}

size_t NetworkManager::Receive(char* buffer, size_t buffer_size) {
    std::lock_guard<std::mutex> lock(receive_buffer_mutex_);
    
    if (receive_buffer_.empty()) {
        return 0;
    }

    size_t copy_size = std::min(buffer_size, receive_buffer_.size());
    std::copy(receive_buffer_.begin(), receive_buffer_.begin() + copy_size, buffer);
    receive_buffer_.erase(receive_buffer_.begin(), receive_buffer_.begin() + copy_size);
    
    return copy_size;
}

void NetworkManager::SetMessageCallback(MessageCallback callback) {
    message_callback_ = callback;
}

void NetworkManager::Update() {
    // Called from game loop
    // Process any callbacks
}

void NetworkManager::Shutdown() {
    if (impl_ && connected_) {
        Disconnect();
    }
    impl_.reset();
    std::cout << "NetworkManager shutdown" << std::endl;
}

void NetworkManager::ReceiveThreadFunc() {
    const size_t BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];

    while (impl_ && impl_->running) {
        if (protocol_type_ == ProtocolType::UDP && impl_->udp_socket) {
            std::string from_addr;
            uint16_t from_port;
            int bytes_received = impl_->udp_socket->ReceiveFrom(buffer, BUFFER_SIZE, from_addr, from_port);
            
            if (bytes_received > 0) {
                {
                    std::lock_guard<std::mutex> lock(receive_buffer_mutex_);
                    receive_buffer_.insert(receive_buffer_.end(), buffer, buffer + bytes_received);
                }
                
                if (message_callback_) {
                    message_callback_(std::string(buffer, bytes_received), bytes_received);
                }
            }
        } else if (protocol_type_ == ProtocolType::TCP) {
            if (impl_->tcp_socket) {
                int bytes_received = impl_->tcp_socket->Receive(buffer, BUFFER_SIZE);
                
                if (bytes_received > 0) {
                    {
                        std::lock_guard<std::mutex> lock(receive_buffer_mutex_);
                        receive_buffer_.insert(receive_buffer_.end(), buffer, buffer + bytes_received);
                    }
                    
                    if (message_callback_) {
                        message_callback_(std::string(buffer, bytes_received), bytes_received);
                    }
                } else if (bytes_received == 0) {
                    // Connection closed
                    std::cout << "TCP connection closed by peer" << std::endl;
                    break;
                }
            }
        }
        
        // Small sleep to prevent spinning
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

} // namespace TopSpeed
