#include <SFML/Graphics.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { std::cerr << "Socket error\n"; return 1; }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (::bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed\n"; return 1;
    }
    if (listen(server_fd, 5) < 0) { std::cerr << "Listen failed\n"; return 1; }

    std::cout << "Server listening on port 8080...\n";

    int addrlen = sizeof(address);
    int client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    if (client_fd < 0) { std::cerr << "Accept failed\n"; return 1; }

    std::cout << "Client connected! Opening a new window...\n";

    // --- SFML window ---
    sf::RenderWindow window(sf::VideoMode(400, 200), "New Window");
    sf::Font font;
    if (!font.loadFromFile("/System/Library/Fonts/SFNSDisplay.ttf")) {
        std::cerr << "Font load failed\n";
    }

    sf::Text text("Hello! Client connected!", font, 20);
    text.setFillColor(sf::Color::White);
    text.setPosition(20, 80);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        window.clear(sf::Color::Black);
        window.draw(text);
        window.display();
    }

    close(client_fd);
    close(server_fd);
}
