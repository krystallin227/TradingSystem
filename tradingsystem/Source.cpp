#include <iostream>
#include <fstream>
#include <string>
#include <winsock2.h>
#include <Ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

class FileSocketConnector {
private:
    SOCKET server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    std::string filename;

public:
    FileSocketConnector(std::string file, int port) : filename(std::move(file)) {
        WSADATA wsaData;
        int result;

        // Initialize Winsock
        result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << "WSAStartup failed: " << result << std::endl;
            exit(EXIT_FAILURE);
        }

        // Create socket file descriptor
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == INVALID_SOCKET) {
            std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        // Bind the socket to the network address and port
        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
            std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
            closesocket(server_fd);
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        // Listen for incoming connections
        if (listen(server_fd, 3) == SOCKET_ERROR) {
            std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
            closesocket(server_fd);
            WSACleanup();
            exit(EXIT_FAILURE);
        }
    }

    ~FileSocketConnector() {
        closesocket(client_socket);
        closesocket(server_fd);
        WSACleanup();
    }

    void subscribe() {
        // Accept a connection
        client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
            closesocket(server_fd);
            WSACleanup();
            exit(EXIT_FAILURE);
        }
    }

    void publish() {
        std::ifstream file(filename);
        std::string line;

        if (file.is_open()) {
            while (getline(file, line)) {
                std::cout << "Sending line: " << line << std::endl;  // Print the line
                send(client_socket, line.c_str(), static_cast<int>(line.size()), 0);
            }
            file.close();
        }
        else {
            std::cerr << "Unable to open file\n";
        }
    }
};

int main() {
    FileSocketConnector connector("trades.txt", 12345);
    connector.subscribe();
    connector.publish();
    return 0;
}
