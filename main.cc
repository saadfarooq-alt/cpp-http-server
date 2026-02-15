#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <ctime>

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (::bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed\n";
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        std::cerr << "Listen failed\n";
        return 1;
    }

    std::cout << "Server listening on port 8080...\n";

    int addrlen = sizeof(address);
    int client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    if (client_fd < 0) {
        std::cerr << "Accept failed\n";
        return 1;
    }

    std::cout << "Client connected!\n";

    // Seed random number for the game
    std::srand(std::time(nullptr));
    int secret = std::rand() % 100 + 1;

    char buffer[1024] = {0};
    const char* msg = "Guess a number between 1 and 100:\n";
    send(client_fd, msg, strlen(msg), 0);

    while (true) {
        int valread = read(client_fd, buffer, sizeof(buffer) - 1);
        if (valread <= 0) break; // client disconnected

        buffer[valread] = '\0'; // null-terminate string
        int guess = atoi(buffer);

        if (guess < secret) send(client_fd, "Too low!\n", 9, 0);
        else if (guess > secret) send(client_fd, "Too high!\n", 10, 0);
        else {
            send(client_fd, "Correct! You win!\n", 18, 0);
            secret = std::rand() % 100 + 1; // reset game
            send(client_fd, "Guess a number between 1 and 100:\n", 35, 0);
        }

        memset(buffer, 0, sizeof(buffer));
    }

    close(client_fd);
    close(server_fd);
}
