#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    std::cout << "Socket created successfully" << std::endl;

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    // Use global ::bind to avoid conflict with std::bind
    if (::bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        std::cerr << "Listen failed" << std::endl;
        return 1;
    }

    std::cout << "Server listening on port 8080..." << std::endl;

    int addrlen = sizeof(address);
    int client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    if (client_fd < 0) {
        std::cerr << "Accept failed" << std::endl;
        return 1;
    }

    std::cout << "Client connected!" << std::endl;

    char buffer[1024] = {0};
    const char* msg = "Hello from server!\n";
    send(client_fd, msg, strlen(msg), 0);

    close(client_fd);
    close(server_fd);
}
