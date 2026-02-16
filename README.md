# Hybrid Dress-Up Game

A real-time, networked dress-up game built with C++ and SFML, featuring a desktop visualization and web-based control interface. Multiple users can collaboratively dress a virtual mannequin through their web browsers while watching changes appear instantly on the server's SFML window.

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![C++](https://img.shields.io/badge/C++-17-00599C?logo=c%2B%2B)
![SFML](https://img.shields.io/badge/SFML-3.0.2-green)

## Features

- **Real-time Synchronization** - Web changes appear instantly on the desktop app
- **Multi-user Support** - Multiple clients can control the same mannequin simultaneously
- **Thread-safe** - Concurrent requests handled safely with mutex locks
- **Beautiful Web UI** - Modern gradient design with responsive layout
- **Desktop Visualization** - SFML-powered graphical display of the dressed mannequin
- **Auto-refresh** - Web dashboard updates every 2 seconds
- **HTTP Server** - Built-in lightweight HTTP server (no external dependencies)

## Demo

### Desktop View (SFML)
The server window displays the mannequin with clothing items rendered in real-time.

### Web Dashboard
Access `http://localhost:8080` in any browser to control the outfit with a click.

```
┌─────────────────┐          ┌──────────────────┐
│  Web Browser    │ ──────▶ │   HTTP Server     │
│  localhost:8080 │          │   (Port 8080)    │
└─────────────────┘          └────────┬─────────┘
                                      │
                                      ▼
                            ┌────────────────────┐
                            │  Shared State      │
                            │  (Thread-safe)     │
                            └────────┬───────────┘
                                     │
                                     ▼
                            ┌────────────────────┐
                            │  SFML Window       │
                            │  (Main Thread)     │
                            └────────────────────┘
```

## Getting Started

### Prerequisites

- **macOS** (tested on macOS 15.7.4)
- **Homebrew** package manager
- **SFML 3.0.2** or higher
- **C++17** compatible compiler (g++ or clang++)

### Installation

1. **Install SFML via Homebrew**
   ```bash
   brew install sfml
   ```

2. **Clone the repository**
   ```bash
   git clone https://github.com/saadfarooq-alt/cpp-http-server.git
   cd cpp-http-server
   ```

3. **Compile the project**
   ```bash
   g++ -std=c++17 hybrid_dressup.cc -o hybrid_dressup \
       -I/opt/homebrew/include \
       -L/opt/homebrew/lib \
       -lsfml-graphics -lsfml-window -lsfml-system -pthread
   ```

### Running the Game

1. **Start the server**
   ```bash
   ./hybrid_dressup
   ```
   
   You should see:
   ```
   HTTP Server running at http://localhost:8080
   Open the URL in your browser to control the outfit!
   ```

2. **Open the web dashboard**
   
   Navigate to `http://localhost:8080` in any web browser (Chrome, Safari, Firefox, etc.)

3. **Dress the mannequin!**
   
   Click clothing items in the web interface and watch them appear on the SFML window in real-time.

## Available Clothing

| Category | Items |
|----------|-------|
| **Hats** | Red Hat, Blue Hat |
| **Shirts** | Red Shirt, Green Shirt, Blue Shirt |
| **Pants** | Black Pants, Blue Jeans, Khaki Pants |
| **Shoes** | White Sneakers, Black Boots |

## How It Works

### Architecture Overview

The project consists of three main components:

#### 1. **HTTP Server** (Background Thread)
- Listens on port 8080
- Serves the HTML dashboard
- Handles REST-like endpoints:
  - `GET /` - Returns the web dashboard
  - `GET /outfit` - Returns current outfit as JSON
  - `GET /wear?type=X&item=Y` - Updates a clothing item
  - `GET /reset` - Removes all clothing

#### 2. **Shared State** (Thread-safe)
```cpp
struct OutfitState {
    std::string hat = "none";
    std::string shirt = "none";
    std::string pants = "none";
    std::string shoes = "none";
    std::mutex mtx;  // Protects concurrent access
};
```

#### 3. **SFML Window** (Main Thread)
- Polls outfit state every frame
- Renders the mannequin with current clothing
- Updates clothing colors based on item names

### Key Technical Details

- **Thread Safety**: All outfit state access is protected by mutex locks
- **Non-blocking**: HTTP requests are handled in detached threads
- **SFML 3 Compatibility**: Uses proper vector initialization syntax
- **Zero External Dependencies**: HTTP server built from scratch using BSD sockets

## API Endpoints

### `GET /`
Returns the HTML web dashboard.

### `GET /outfit`
Returns the current outfit state as JSON.

**Response:**
```json
{
  "hat": "red_hat",
  "shirt": "green_shirt",
  "pants": "blue_jeans",
  "shoes": "sneakers"
}
```

### `GET /wear?type=<TYPE>&item=<ITEM>`
Updates a specific clothing item.

**Parameters:**
- `type`: One of `hat`, `shirt`, `pants`, `shoes`
- `item`: Item name (e.g., `red_hat`, `blue_jeans`, `none`)

**Example:**
```
http://localhost:8080/wear?type=shirt&item=red_shirt
```

### `GET /reset`
Removes all clothing items (sets everything to `none`).

## Use Cases

- **Collaborative Fashion Design** - Team members vote on outfit combinations
- **Educational Tool** - Teach networking concepts with a visual application
- **Game Development Learning** - Study client-server architecture
- **Remote Dress-up Party** - Friends dress a character together online
- **UI/UX Prototyping** - Test real-time web-to-desktop synchronization

## Customization

### Adding New Clothing Items

1. **Update the web dashboard** (`getDashboardHTML()`)
   ```html
   <button class="item-btn purple" onclick="wear('hat', 'purple_hat')">
       Purple Hat
   </button>
   ```

2. **Add color mapping** (`getColorFromItem()`)
   ```cpp
   if (item.find("purple") != std::string::npos) 
       return sf::Color(128, 0, 128);
   ```

### Changing the Port

Modify the `PORT` constant:
```cpp
constexpr int PORT = 9000;  // Change to your desired port
```

### Customizing the Mannequin

Adjust the body shapes in the `main()` function:
```cpp
sf::RectangleShape head({100.f, 100.f});  // Bigger head
head.setPosition({240.f, 130.f});         // Different position
```

## Troubleshooting

### "Bind failed" error
The port is already in use. Either:
- Kill the process using port 8080: `lsof -ti:8080 | xargs kill -9`
- Change the `PORT` constant in the code

### SFML window crashes on startup
Make sure you're running on the **main thread**. SFML windows must be created on the main thread on macOS.

### Font not found
The code uses `/System/Library/Fonts/Helvetica.ttc`. If this doesn't exist on your system, change it to another font path:
```cpp
if (!font.openFromFile("/path/to/your/font.ttf")) {
```

### Web dashboard doesn't update
Check the browser console for errors. The dashboard polls `/outfit` every 2 seconds. Make sure the server is running.

## Learning Resources

This project demonstrates:
- **C++ Threading** - `std::thread`, `std::mutex`, `std::atomic`
- **Socket Programming** - BSD sockets, TCP/IP
- **HTTP Protocol** - Request parsing, response building
- **SFML Graphics** - Shapes, text rendering, event handling
- **Web Development** - HTML, CSS, JavaScript, fetch API
- **Client-Server Architecture** - REST-like endpoints, state synchronization

## Contributing

Contributions are welcome! Here are some ideas:

- [ ] Add image/sprite support instead of colored rectangles
- [ ] Implement WebSocket for true real-time updates
- [ ] Add authentication for multi-user control
- [ ] Create a save/load outfit feature
- [ ] Support for multiple mannequins (rooms)
- [ ] Add drag-and-drop in the web interface
- [ ] Mobile-responsive design improvements
- [ ] Add sound effects when clothing changes

## Acknowledgments

- **SFML Team** - For the excellent multimedia library
- **Homebrew** - For easy package management on macOS
- **C++ Community** - For threading and networking best practices

## Contact

Created by Saad Farooq - feel free to reach out!

- GitHub: [@saadfarooq-alt](https://github.com/saadfarooq-alt)
- Email: s4farooq@uwaterloo.ca

---
