#include "TCPSocket.h"
#include <iostream>
#include <cstring>
#include <utility>

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

// Static Winsock initialization (shared with UDP)
extern class WinsockInit {
public:
    WinsockInit() {
#ifdef _WIN32
        WSADATA wsa_data;
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0) {
            initialized_ = true;
        }
#else
        initialized_ = true;
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

TCPSocket::TCPSocket()
    : socket_handle_(INVALID_SOCKET_VALUE), remote_port_(0) {
}

TCPSocket::TCPSocket(uintptr_t socket, const std::string& remote_addr, uint16_t remote_port)
    : socket_handle_(socket), remote_address_(remote_addr), remote_port_(remote_port) {
}

TCPSocket::~TCPSocket() {
    Close();
}

bool TCPSocket::Create() {
    if (IsOpen()) {
        SetError("Socket already created");
        return false;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
#ifdef _WIN32
        SetError("WSA Error: " + std::to_string(WSAGetLastError()));
#else
        SetError("Socket creation failed");
#endif
        return false;
    }

    socket_handle_ = (uintptr_t)sock;
    std::cout << "TCP Socket created successfully" << std::endl;
    return true;
}

bool TCPSocket::Connect(const std::string& address, uint16_t port) {
    if (!IsOpen()) {
        SetError("Socket not created");
        return false;
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
        hints.ai_socktype = SOCK_STREAM;

        if (getaddrinfo(address.c_str(), nullptr, &hints, &result) != 0) {
            SetError("Invalid address: " + address);
            return false;
        }

        if (result) {
            sockaddr_in* ipv4 = (sockaddr_in*)result->ai_addr;
            addr.sin_addr = ipv4->sin_addr;
            freeaddrinfo(result);
        } else {
            SetError("Could not resolve: " + address);
            return false;
        }
    }

    if (connect((SOCKET)socket_handle_, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
#ifdef _WIN32
        SetError("WSA Connect Error: " + std::to_string(WSAGetLastError()));
#else
        SetError("Connect failed");
#endif
        return false;
    }

    remote_address_ = address;
    remote_port_ = port;
    std::cout << "TCP Socket connected to " << address << ":" << port << std::endl;
    return true;
}

bool TCPSocket::Bind(uint16_t port) {
    if (!IsOpen()) {
        SetError("Socket not created");
        return false;
    }

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    // Allow socket reuse
    int reuse = 1;
    if (setsockopt((SOCKET)socket_handle_, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) == SOCKET_ERROR) {
        std::cerr << "Warning: Could not set SO_REUSEADDR" << std::endl;
    }

    if (bind((SOCKET)socket_handle_, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
#ifdef _WIN32
        SetError("WSA Bind Error: " + std::to_string(WSAGetLastError()));
#else
        SetError("Bind failed");
#endif
        return false;
    }

    std::cout << "TCP Socket bound to port " << port << std::endl;
    return true;
}

bool TCPSocket::Listen(int backlog) {
    if (!IsOpen()) {
        SetError("Socket not created");
        return false;
    }

    if (listen((SOCKET)socket_handle_, backlog) == SOCKET_ERROR) {
#ifdef _WIN32
        SetError("WSA Listen Error: " + std::to_string(WSAGetLastError()));
#else
        SetError("Listen failed");
#endif
        return false;
    }

    std::cout << "TCP Socket listening (backlog: " << backlog << ")" << std::endl;
    return true;
}

TCPSocket* TCPSocket::Accept() {
    if (!IsOpen()) {
        SetError("Socket not created");
        return nullptr;
    }

    sockaddr_in client_addr;
    int addr_len = sizeof(client_addr);
    std::memset(&client_addr, 0, sizeof(client_addr));

    SOCKET client_sock = accept((SOCKET)socket_handle_, (sockaddr*)&client_addr, &addr_len);
    if (client_sock == INVALID_SOCKET) {
#ifdef _WIN32
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK && err != WSAEINPROGRESS) {
            SetError("WSA Accept Error: " + std::to_string(err));
        }
#else
        SetError("Accept failed");
#endif
        return nullptr;
    }

    char addr_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, addr_str, INET_ADDRSTRLEN);
    uint16_t client_port = ntohs(client_addr.sin_port);

    std::cout << "New TCP connection from " << addr_str << ":" << client_port << std::endl;
    return new TCPSocket((uintptr_t)client_sock, addr_str, client_port);
}

int TCPSocket::Send(const char* data, size_t size) {
    if (!IsOpen()) {
        SetError("Socket not open");
        return -1;
    }

    int bytes_sent = send((SOCKET)socket_handle_, data, (int)size, 0);
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

int TCPSocket::Receive(char* buffer, size_t buffer_size) {
    if (!IsOpen()) {
        SetError("Socket not open");
        return -1;
    }

    int bytes_received = recv((SOCKET)socket_handle_, buffer, (int)buffer_size, 0);
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

    return bytes_received;
}

void TCPSocket::Close() {
    if (IsOpen()) {
        shutdown((SOCKET)socket_handle_, SD_BOTH);
        closesocket((SOCKET)socket_handle_);
        socket_handle_ = INVALID_SOCKET_VALUE;
        std::cout << "TCP Socket closed" << std::endl;
    }
}

bool TCPSocket::SetNonBlocking(bool non_blocking) {
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

void TCPSocket::SetError(const std::string& error) {
    last_error_ = error;
    std::cerr << "TCP Error: " << error << std::endl;
}

} // namespace TopSpeed
