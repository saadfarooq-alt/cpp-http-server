#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <netinet/in.h>
#include <unistd.h>

constexpr int PORT = 8080;
std::atomic<bool> running{true};

// Thread-safe queue for new client connections
std::queue<int> clientQueue;
std::mutex queueMutex;

// Store all windows
std::vector<std::unique_ptr<sf::RenderWindow>> windows;

void acceptClients(int serverSocket) {
    while (running) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket >= 0) {
            std::lock_guard<std::mutex> lock(queueMutex);
            clientQueue.push(clientSocket);
            std::cout << "New client connected: " << clientSocket << std::endl;
        }
    }
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
        close(serverSocket);
        return 1;
    }
    
    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Listen failed\n";
        close(serverSocket);
        return 1;
    }
    
    std::cout << "Server listening on port " << PORT << "...\n";
    std::cout << "Connect using: nc localhost " << PORT << std::endl;
    
    // Start accept thread
    std::thread acceptThread(acceptClients, serverSocket);
    
    sf::Font font;
    if (!font.openFromFile("/System/Library/Fonts/Helvetica.ttc")) {
        std::cerr << "Font failed to load\n";
        running = false;
    }
    
    // Main loop - handle windows on main thread
    while (running) {
        // Check for new clients
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            while (!clientQueue.empty()) {
                int clientSocket = clientQueue.front();
                clientQueue.pop();
                
                // Create window on main thread
                auto window = std::make_unique<sf::RenderWindow>(
                    sf::VideoMode({400u, 200u}), 
                    "Client " + std::to_string(clientSocket)
                );
                
                windows.push_back(std::move(window));
            }
        }
        
        // Process all windows
        for (auto it = windows.begin(); it != windows.end();) {
            auto& window = *it;
            
            // Handle events
            while (auto eventOpt = window->pollEvent()) {
                if (eventOpt->is<sf::Event::Closed>()) {
                    window->close();
                }
            }
            
            // Draw
            if (window->isOpen()) {
                window->clear(sf::Color::Black);
                
                sf::Text text(font, "Hello! Client connected!", 20);
                text.setPosition({20.f, 80.f});
                text.setFillColor(sf::Color::White);
                
                window->draw(text);
                window->display();
                ++it;
            } else {
                it = windows.erase(it);
            }
        }
        
        // Exit if no windows and user presses Ctrl+C
        if (windows.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    running = false;
    close(serverSocket);
    
    if (acceptThread.joinable()) {
        acceptThread.join();
    }
    
    return 0;
}
