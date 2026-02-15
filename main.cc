#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

void handleClient(int client_fd, int client_num) {
    // Open SFML 3 window
    sf::RenderWindow window(sf::Vector2u(400, 200), "Client Window");

    sf::Font font;
    if (!font.openFromFile("/System/Library/Fonts/SFNSDisplay.ttf")) {
        std::cerr << "Failed to load font\n";
        close(client_fd);
        return;
    }

    sf::Text text;
    text.setFont(font);
    text.setString("Client " + std::to_string(client_num) + " connected!");
    text.setCharacterSize(20);
    text.setPosition(sf::Vector2f(20.f, 80.f));

    while (window.isOpen()) {
        while (auto eventOpt = window.pollEvent()) {
            const sf::Event& event = *eventOpt;
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear(sf::Color::Black);
        window.draw(text);
        window.display();
    }

    close(client_fd);
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed\n";
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        std::cerr << "Listen failed\n";
        return 1;
    }

    std::cout << "Server listening on port 8080...\n";

    int client_num = 0;
    std::vector<std::thread> threads;

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd >= 0) {
            client_num++;
            // Handle each client in a separate thread
            threads.emplace_back(handleClient, client_fd, client_num);
        }
    }

    // Join threads before exiting (never reached here in this simple example)
    for (auto& t : threads) t.join();
    close(server_fd);
}
