#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

using namespace std;

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        cerr << "Failed to create socket" << endl;
        return 1;
    }

    cout << "Socket created successfully" << endl;

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        cerr << "Bind failed" << endl;
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        cerr << "Listen failed" << endl;
        return 1;
    }

    cout << "Server listening on port 8080..." << endl;

    int addrlen = sizeof(address);
    int client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    if (client_fd < 0) {
        cerr << "Accept failed" << endl;
        return 1;
    }

    cout << "Client connected!" << endl;

    // Simple game: guess a number
    int secret = rand() % 100 + 1;
    char buffer[1024] = {0};
    send(client_fd, "Guess a number 1-100:\n", 23, 0);

    while (true) {
        int valread = read(client_fd, buffer, 1024);
        if (valread <= 0) break;

        int guess = atoi(buffer);
        if (guess < secret) send(client_fd, "Too low!\n", 9, 0);
        else if (guess > secret) send(client_fd, "Too high!\n", 10, 0);
        else {
            send(client_fd, "Correct! You win!\n", 18, 0);
            secret = rand() % 100 + 1; // Reset game
        }
        memset(buffer, 0, sizeof(buffer));
    }

    close(client_fd);
    close(server_fd);
}
