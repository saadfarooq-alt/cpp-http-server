#include <SFML/Graphics.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <map>
#include <mutex>
#include <atomic>
#include <sstream>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

constexpr int PORT = 8080;
std::atomic<bool> running{true};

// Thread-safe outfit state
struct OutfitState {
    std::string hat = "none";
    std::string shirt = "none";
    std::string pants = "none";
    std::string shoes = "none";
    std::mutex mtx;
    
    void set(const std::string& type, const std::string& item) {
        std::lock_guard<std::mutex> lock(mtx);
        if (type == "hat") hat = item;
        else if (type == "shirt") shirt = item;
        else if (type == "pants") pants = item;
        else if (type == "shoes") shoes = item;
    }
    
    std::map<std::string, std::string> getAll() {
        std::lock_guard<std::mutex> lock(mtx);
        return {
            {"hat", hat},
            {"shirt", shirt},
            {"pants", pants},
            {"shoes", shoes}
        };
    }
    
    std::string toJSON() {
        std::lock_guard<std::mutex> lock(mtx);
        std::ostringstream json;
        json << "{";
        json << "\"hat\":\"" << hat << "\",";
        json << "\"shirt\":\"" << shirt << "\",";
        json << "\"pants\":\"" << pants << "\",";
        json << "\"shoes\":\"" << shoes << "\"";
        json << "}";
        return json.str();
    }
};

OutfitState globalOutfit;

// Simple HTTP response builder
std::string buildHTTPResponse(const std::string& body, const std::string& contentType = "text/html") {
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;
    return response.str();
}

// HTML dashboard
std::string getDashboardHTML() {
    return R"(<!DOCTYPE html>
<html>
<head>
    <title>Dress-Up Dashboard</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            border-radius: 20px;
            padding: 30px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
        }
        h1 {
            text-align: center;
            color: #333;
            margin-bottom: 10px;
            font-size: 2.5em;
        }
        .subtitle {
            text-align: center;
            color: #666;
            margin-bottom: 30px;
            font-size: 1.1em;
        }
        .current-outfit {
            background: #f8f9fa;
            padding: 20px;
            border-radius: 10px;
            margin-bottom: 30px;
        }
        .current-outfit h2 {
            color: #333;
            margin-bottom: 15px;
        }
        .outfit-item {
            display: flex;
            justify-content: space-between;
            padding: 10px;
            margin: 5px 0;
            background: white;
            border-radius: 5px;
            border-left: 4px solid #667eea;
        }
        .outfit-item strong {
            color: #667eea;
            text-transform: uppercase;
            font-size: 0.9em;
        }
        .outfit-item span {
            color: #333;
            font-weight: 600;
        }
        .wardrobe {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
        }
        .category {
            background: #f8f9fa;
            padding: 20px;
            border-radius: 10px;
        }
        .category h3 {
            color: #333;
            margin-bottom: 15px;
            font-size: 1.3em;
            border-bottom: 2px solid #667eea;
            padding-bottom: 10px;
        }
        .item-btn {
            width: 100%;
            padding: 12px;
            margin: 8px 0;
            border: 2px solid #ddd;
            border-radius: 8px;
            background: white;
            cursor: pointer;
            transition: all 0.3s;
            font-size: 1em;
            font-weight: 500;
        }
        .item-btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 4px 12px rgba(0,0,0,0.15);
            border-color: #667eea;
        }
        .item-btn.active {
            background: #667eea;
            color: white;
            border-color: #667eea;
        }
        .red { background: #ff6b6b !important; color: white; }
        .blue { background: #4dabf7 !important; color: white; }
        .green { background: #51cf66 !important; color: white; }
        .black { background: #333 !important; color: white; }
        .white { background: #f8f9fa !important; border: 2px solid #ddd; }
        .khaki { background: #c3b091 !important; color: white; }
        .denim { background: #0064c8 !important; color: white; }
        .reset-btn {
            width: 100%;
            padding: 15px;
            margin-top: 20px;
            background: #ff6b6b;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 1.1em;
            font-weight: bold;
            cursor: pointer;
            transition: all 0.3s;
        }
        .reset-btn:hover {
            background: #ee5a52;
            transform: translateY(-2px);
            box-shadow: 0 4px 12px rgba(255,107,107,0.4);
        }
        .status {
            text-align: center;
            padding: 10px;
            margin-top: 15px;
            border-radius: 5px;
            font-weight: 600;
            display: none;
        }
        .status.success {
            background: #d3f9d8;
            color: #2b8a3e;
            display: block;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>👔 Dress-Up Dashboard</h1>
        <p class="subtitle">Control the outfit in real-time • Changes appear instantly on the desktop app</p>
        
        <div class="current-outfit">
            <h2>Current Outfit</h2>
            <div class="outfit-item">
                <strong>🎩 Hat:</strong>
                <span id="current-hat">none</span>
            </div>
            <div class="outfit-item">
                <strong>👕 Shirt:</strong>
                <span id="current-shirt">none</span>
            </div>
            <div class="outfit-item">
                <strong>👖 Pants:</strong>
                <span id="current-pants">none</span>
            </div>
            <div class="outfit-item">
                <strong>👟 Shoes:</strong>
                <span id="current-shoes">none</span>
            </div>
        </div>

        <div class="wardrobe">
            <div class="category">
                <h3>🎩 Hats</h3>
                <button class="item-btn red" onclick="wear('hat', 'red_hat')">Red Hat</button>
                <button class="item-btn blue" onclick="wear('hat', 'blue_hat')">Blue Hat</button>
                <button class="item-btn" onclick="wear('hat', 'none')">Remove Hat</button>
            </div>

            <div class="category">
                <h3>👕 Shirts</h3>
                <button class="item-btn red" onclick="wear('shirt', 'red_shirt')">Red Shirt</button>
                <button class="item-btn green" onclick="wear('shirt', 'green_shirt')">Green Shirt</button>
                <button class="item-btn blue" onclick="wear('shirt', 'blue_shirt')">Blue Shirt</button>
                <button class="item-btn" onclick="wear('shirt', 'none')">Remove Shirt</button>
            </div>

            <div class="category">
                <h3>👖 Pants</h3>
                <button class="item-btn black" onclick="wear('pants', 'black_pants')">Black Pants</button>
                <button class="item-btn denim" onclick="wear('pants', 'blue_jeans')">Blue Jeans</button>
                <button class="item-btn khaki" onclick="wear('pants', 'khaki_pants')">Khaki Pants</button>
                <button class="item-btn" onclick="wear('pants', 'none')">Remove Pants</button>
            </div>

            <div class="category">
                <h3>👟 Shoes</h3>
                <button class="item-btn white" onclick="wear('shoes', 'sneakers')">White Sneakers</button>
                <button class="item-btn black" onclick="wear('shoes', 'boots')">Black Boots</button>
                <button class="item-btn" onclick="wear('shoes', 'none')">Remove Shoes</button>
            </div>
        </div>

        <button class="reset-btn" onclick="resetOutfit()">🔄 Reset All</button>
        <div id="status" class="status"></div>
    </div>

    <script>
        function wear(type, item) {
            fetch('/wear?type=' + type + '&item=' + item)
                .then(response => response.text())
                .then(data => {
                    updateOutfit();
                    showStatus('Updated!');
                });
        }

        function resetOutfit() {
            fetch('/reset')
                .then(response => response.text())
                .then(data => {
                    updateOutfit();
                    showStatus('Outfit reset!');
                });
        }

        function updateOutfit() {
            fetch('/outfit')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('current-hat').textContent = data.hat;
                    document.getElementById('current-shirt').textContent = data.shirt;
                    document.getElementById('current-pants').textContent = data.pants;
                    document.getElementById('current-shoes').textContent = data.shoes;
                });
        }

        function showStatus(msg) {
            const status = document.getElementById('status');
            status.textContent = msg;
            status.className = 'status success';
            setTimeout(() => {
                status.style.display = 'none';
            }, 2000);
        }

        // Update outfit display every 2 seconds
        setInterval(updateOutfit, 2000);
        updateOutfit();
    </script>
</body>
</html>)";
}

// Parse HTTP request
void parseRequest(const std::string& request, std::string& method, std::string& path, std::map<std::string, std::string>& params) {
    std::istringstream iss(request);
    iss >> method >> path;
    
    // Parse query parameters
    size_t qPos = path.find('?');
    if (qPos != std::string::npos) {
        std::string query = path.substr(qPos + 1);
        path = path.substr(0, qPos);
        
        std::istringstream qss(query);
        std::string pair;
        while (std::getline(qss, pair, '&')) {
            size_t eqPos = pair.find('=');
            if (eqPos != std::string::npos) {
                params[pair.substr(0, eqPos)] = pair.substr(eqPos + 1);
            }
        }
    }
}

// Handle HTTP requests
void handleHTTPClient(int clientSocket) {
    char buffer[4096];
    ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
    
    if (bytesRead <= 0) {
        close(clientSocket);
        return;
    }
    
    buffer[bytesRead] = '\0';
    std::string request(buffer);
    
    std::string method, path;
    std::map<std::string, std::string> params;
    parseRequest(request, method, path, params);
    
    std::string response;
    
    if (path == "/" || path == "/index.html") {
        response = buildHTTPResponse(getDashboardHTML());
    }
    else if (path == "/outfit") {
        response = buildHTTPResponse(globalOutfit.toJSON(), "application/json");
    }
    else if (path == "/wear") {
        if (params.count("type") && params.count("item")) {
            globalOutfit.set(params["type"], params["item"]);
            response = buildHTTPResponse("OK");
        } else {
            response = buildHTTPResponse("Missing parameters");
        }
    }
    else if (path == "/reset") {
        globalOutfit.set("hat", "none");
        globalOutfit.set("shirt", "none");
        globalOutfit.set("pants", "none");
        globalOutfit.set("shoes", "none");
        response = buildHTTPResponse("OK");
    }
    else {
        response = buildHTTPResponse("404 Not Found");
    }
    
    write(clientSocket, response.c_str(), response.length());
    close(clientSocket);
}

// HTTP server thread
void runHTTPServer() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Failed to create socket\n";
        return;
    }
    
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Bind failed\n";
        close(serverSocket);
        return;
    }
    
    if (listen(serverSocket, 10) < 0) {
        std::cerr << "Listen failed\n";
        close(serverSocket);
        return;
    }
    
    std::cout << "🌐 HTTP Server running at http://localhost:" << PORT << std::endl;
    std::cout << "📱 Open the URL in your browser to control the outfit!\n" << std::endl;
    
    while (running) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket >= 0) {
            std::thread(handleHTTPClient, clientSocket).detach();
        }
    }
    
    close(serverSocket);
}

// Get color from item name
sf::Color getColorFromItem(const std::string& item) {
    if (item.find("red") != std::string::npos) return sf::Color::Red;
    if (item.find("blue") != std::string::npos) return sf::Color::Blue;
    if (item.find("green") != std::string::npos) return sf::Color::Green;
    if (item.find("black") != std::string::npos) return sf::Color::Black;
    if (item.find("white") != std::string::npos) return sf::Color::White;
    if (item.find("khaki") != std::string::npos) return sf::Color(195, 176, 145);
    if (item.find("denim") != std::string::npos || item.find("jeans") != std::string::npos) return sf::Color(0, 100, 200);
    return sf::Color::Transparent;
}

int main() {
    // Start HTTP server in background
    std::thread httpThread(runHTTPServer);
    
    // Create SFML window
    sf::RenderWindow window(sf::VideoMode({600u, 700u}), "Dress-Up Game - Server View");
    window.setFramerateLimit(30);
    
    sf::Font font;
    if (!font.openFromFile("/System/Library/Fonts/Helvetica.ttc")) {
        std::cerr << "Failed to load font\n";
        running = false;
        return 1;
    }
    
    // Body model
    sf::RectangleShape head({80.f, 80.f});
    head.setPosition(260.f, 150.f);
    head.setFillColor(sf::Color(255, 220, 177));
    head.setOutlineThickness(3.f);
    head.setOutlineColor(sf::Color::Black);
    
    sf::RectangleShape torso({100.f, 120.f});
    torso.setPosition(250.f, 230.f);
    torso.setFillColor(sf::Color(255, 220, 177));
    torso.setOutlineThickness(3.f);
    torso.setOutlineColor(sf::Color::Black);
    
    sf::RectangleShape legs({100.f, 150.f});
    legs.setPosition(250.f, 350.f);
    legs.setFillColor(sf::Color(255, 220, 177));
    legs.setOutlineThickness(3.f);
    legs.setOutlineColor(sf::Color::Black);
    
    // Clothing shapes
    sf::RectangleShape hatShape({80.f, 50.f});
    hatShape.setPosition(260.f, 110.f);
    hatShape.setOutlineThickness(2.f);
    hatShape.setOutlineColor(sf::Color::Black);
    
    sf::RectangleShape shirtShape({100.f, 120.f});
    shirtShape.setPosition(250.f, 230.f);
    shirtShape.setOutlineThickness(2.f);
    shirtShape.setOutlineColor(sf::Color::Black);
    
    sf::RectangleShape pantsShape({100.f, 150.f});
    pantsShape.setPosition(250.f, 350.f);
    pantsShape.setOutlineThickness(2.f);
    pantsShape.setOutlineColor(sf::Color::Black);
    
    sf::RectangleShape shoesShape({90.f, 50.f});
    shoesShape.setPosition(255.f, 500.f);
    shoesShape.setOutlineThickness(2.f);
    shoesShape.setOutlineColor(sf::Color::Black);
    
    // Title
    sf::Text title(font, "Dress-Up Game - Server View", 28);
    title.setPosition(80.f, 20.f);
    title.setFillColor(sf::Color::White);
    
    sf::Text subtitle(font, "Control via http://localhost:8080", 18);
    subtitle.setPosition(140.f, 60.f);
    subtitle.setFillColor(sf::Color(200, 200, 200));
    
    // Status text
    sf::Text statusText(font, "", 16);
    statusText.setPosition(50.f, 600.f);
    statusText.setFillColor(sf::Color::White);
    
    while (window.isOpen()) {
        while (auto eventOpt = window.pollEvent()) {
            if (eventOpt->is<sf::Event::Closed>()) {
                window.close();
                running = false;
            }
        }
        
        // Get current outfit
        auto outfit = globalOutfit.getAll();
        
        // Update clothing colors
        hatShape.setFillColor(outfit["hat"] != "none" ? getColorFromItem(outfit["hat"]) : sf::Color::Transparent);
        shirtShape.setFillColor(outfit["shirt"] != "none" ? getColorFromItem(outfit["shirt"]) : sf::Color::Transparent);
        pantsShape.setFillColor(outfit["pants"] != "none" ? getColorFromItem(outfit["pants"]) : sf::Color::Transparent);
        shoesShape.setFillColor(outfit["shoes"] != "none" ? getColorFromItem(outfit["shoes"]) : sf::Color::Transparent);
        
        // Update status
        std::ostringstream status;
        status << "Hat: " << outfit["hat"] << "  |  Shirt: " << outfit["shirt"] << "\n";
        status << "Pants: " << outfit["pants"] << "  |  Shoes: " << outfit["shoes"];
        statusText.setString(status.str());
        
        // Draw
        window.clear(sf::Color(50, 50, 70));
        
        window.draw(title);
        window.draw(subtitle);
        
        // Draw body
        window.draw(legs);
        window.draw(torso);
        window.draw(head);
        
        // Draw clothing
        if (outfit["shoes"] != "none") window.draw(shoesShape);
        if (outfit["pants"] != "none") window.draw(pantsShape);
        if (outfit["shirt"] != "none") window.draw(shirtShape);
        if (outfit["hat"] != "none") window.draw(hatShape);
        
        window.draw(statusText);
        
        window.display();
    }
    
    running = false;
    if (httpThread.joinable()) {
        httpThread.join();
    }
    
    return 0;
}
