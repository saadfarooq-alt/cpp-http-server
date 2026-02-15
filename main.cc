#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <netinet/in.h>
#include <unistd.h>

constexpr int PORT = 8080;
std::atomic<bool> running{true};

void handleClient(int clientSocket) {
    sf::VideoMode mode({400u, 200u}); // Construct with Vector2u
    sf::RenderWindow window(mode, "New Client Window");

    sf::Font font;
    if (!font.openFromFile("/System/Library/Fonts/SFNSDisplay.ttf")) {
        std::cerr << "Font failed to load\n";
        close(clientSocket);
        return;
    }

    sf::Text text(font, "Hello! Client connected!", 20);
    text.setPosition({20.f, 80.f});

    while (window.isOpen() && running) {
        // Poll events
        while (auto eventOpt = window.pollEvent()) {
            const sf::Event& event = *eventOpt;

            // SFML 3: event.type -> check Closed variant
            if (std::holds_alternative<sf::Event::Closed>(event)) {
                window.close();
            }
        }

        window.clear(sf::Color::Black);
        window.draw(text);
        window.display();
    }

    close(clientSocket);
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Bind failed\n";
        return 1;
    }

    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Listen failed\n";
        return 1;
    }

    std::cout << "Server listening on port " << PORT << "...\n";

    std::vector<std::thread> clients;

    while (running) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket >= 0) {
            clients.emplace_back(handleClient, clientSocket);
        }
    }

    for (auto& t : clients) {
        if (t.joinable())
            t.join();
    }

    close(serverSocket);
    return 0;
}
