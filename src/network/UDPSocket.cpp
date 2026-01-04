#include "UDPSocket.h"
#include <iostream>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
    #pragma comment(lib, "Mswsock.lib")
    #pragma comment(lib, "AdvApi32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <fcntl.h>
    #define closesocket close
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

namespace TopSpeed {

// Static Winsock initialization
class WinsockInit {
public:
    WinsockInit() {
#ifdef _WIN32
        WSADATA wsa_data;
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0) {
            initialized_ = true;
        }
#else
        initialized_ = true;  // Unix doesn't need socket initialization
#endif
    }

    ~WinsockInit() {
#ifdef _WIN32
        if (initialized_) {
            WSACleanup();
        }
#endif
    }

private:
    static bool initialized_;
};

bool WinsockInit::initialized_ = false;
static WinsockInit winsock_init_;  // Global initialization

UDPSocket::UDPSocket()
    : socket_handle_(INVALID_SOCKET_VALUE) {
}

UDPSocket::~UDPSocket() {
    Close();
}

bool UDPSocket::Create() {
    if (IsOpen()) {
        SetError("Socket already created");
        return false;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
#ifdef _WIN32
        SetError("WSA Error: " + std::to_string(WSAGetLastError()));
#else
        SetError("Socket creation failed");
#endif
        return false;
    }

    socket_handle_ = (uintptr_t)sock;
    std::cout << "UDP Socket created successfully" << std::endl;
    return true;
}

bool UDPSocket::Bind(uint16_t port) {
    if (!IsOpen()) {
        SetError("Socket not created");
        return false;
    }

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind((SOCKET)socket_handle_, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
#ifdef _WIN32
        SetError("WSA Error: " + std::to_string(WSAGetLastError()));
#else
        SetError("Bind failed");
#endif
        return false;
    }

    std::cout << "UDP Socket bound to port " << port << std::endl;
    return true;
}

int UDPSocket::SendTo(const std::string& address, uint16_t port, const char* data, size_t size) {
    if (!IsOpen()) {
        SetError("Socket not created");
        return -1;
    }

    // Resolve hostname/IP
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, address.c_str(), &addr.sin_addr) <= 0) {
        // Try hostname resolution
        struct addrinfo hints, *result;
        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;

        if (getaddrinfo(address.c_str(), nullptr, &hints, &result) != 0) {
            SetError("Invalid address: " + address);
            return -1;
        }

        if (result) {
            sockaddr_in* ipv4 = (sockaddr_in*)result->ai_addr;
            addr.sin_addr = ipv4->sin_addr;
            freeaddrinfo(result);
        } else {
            SetError("Could not resolve: " + address);
            return -1;
        }
    }

    int bytes_sent = sendto((SOCKET)socket_handle_, data, (int)size, 0, (sockaddr*)&addr, sizeof(addr));
    if (bytes_sent == SOCKET_ERROR) {
#ifdef _WIN32
        SetError("WSA Send Error: " + std::to_string(WSAGetLastError()));
#else
        SetError("Send failed");
#endif
        return -1;
    }

    return bytes_sent;
}

int UDPSocket::ReceiveFrom(char* buffer, size_t buffer_size, std::string& from_address, uint16_t& from_port) {
    if (!IsOpen()) {
        SetError("Socket not created");
        return -1;
    }

    sockaddr_in addr;
    int addr_len = sizeof(addr);
    std::memset(&addr, 0, sizeof(addr));

    int bytes_received = recvfrom((SOCKET)socket_handle_, buffer, (int)buffer_size, 0,
                                   (sockaddr*)&addr, &addr_len);

    if (bytes_received == SOCKET_ERROR) {
#ifdef _WIN32
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK && err != WSAEINPROGRESS) {
            SetError("WSA Receive Error: " + std::to_string(err));
            return -1;
        }
#else
        SetError("Receive failed");
        return -1;
#endif
    }

    // Extract sender info
    char addr_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, addr_str, INET_ADDRSTRLEN);
    from_address = addr_str;
    from_port = ntohs(addr.sin_port);

    return bytes_received;
}

void UDPSocket::Close() {
    if (IsOpen()) {
        closesocket((SOCKET)socket_handle_);
        socket_handle_ = INVALID_SOCKET_VALUE;
        std::cout << "UDP Socket closed" << std::endl;
    }
}

bool UDPSocket::SetNonBlocking(bool non_blocking) {
    if (!IsOpen()) {
        SetError("Socket not created");
        return false;
    }

#ifdef _WIN32
    u_long mode = non_blocking ? 1 : 0;
    if (ioctlsocket((SOCKET)socket_handle_, FIONBIO, &mode) == SOCKET_ERROR) {
        SetError("WSA Error: " + std::to_string(WSAGetLastError()));
        return false;
    }
#else
    int flags = fcntl((SOCKET)socket_handle_, F_GETFL, 0);
    if (flags == -1) {
        SetError("fcntl failed");
        return false;
    }
    if (non_blocking) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    if (fcntl((SOCKET)socket_handle_, F_SETFL, flags) == -1) {
        SetError("fcntl failed");
        return false;
    }
#endif

    return true;
}

void UDPSocket::SetError(const std::string& error) {
    last_error_ = error;
    std::cerr << "UDP Error: " << error << std::endl;
}

} // namespace TopSpeed
