# C++ WebSocket Chat Server with React UI

## Overview
This project implements a real-time chat application featuring a C++ WebSocket server and a React-based web application for the user interface. The server handles message broadcasting and client management, while the React UI provides a user-friendly way to send and receive messages.

## Architecture
The system consists of two main components:
1.  **C++ WebSocket Server:** Built using Boost.Asio and Boost.Beast for handling WebSocket connections, managing client sessions, and broadcasting messages in JSON format.
2.  **React UI Client:** A single-page application created with Create React App, providing the user interface. It communicates with the C++ server using the browser's native WebSocket API.

All communication between the client and server is done via JSON messages adhering to a defined protocol.

## Features
*   **Real-time Messaging:** Send and receive messages instantly.
*   **JSON Message Protocol:** Structured communication between server and client.
*   **Client Connection/Disconnection Notifications:** Users are notified when other users join or leave.
*   **Basic UI:** Message display area, input field, and connection status indicator.
*   **C++ Server:** Efficiently handles multiple client connections using asynchronous operations.
*   **React Client:** Modern, component-based UI.

## Message Protocol
The server and client communicate using JSON messages. Key message types include:

*   **`client_send_message`**
    *   **Direction:** React UI -> C++ Server
    *   **Purpose:** Sent when a user types and sends a message.
    *   **Payload Example:** `{"type": "client_send_message", "payload": {"text": "Hello everyone!"}}`

*   **`server_broadcast_message`**
    *   **Direction:** C++ Server -> React UI (all connected clients)
    *   **Purpose:** Broadcasts a user's message to all clients.
    *   **Payload Example:** `{"type": "server_broadcast_message", "payload": {"user_id": "sess_xxxx", "text": "Hello everyone!", "timestamp": "2023-10-27T10:30:00Z"}}`

*   **`server_client_connected`**
    *   **Direction:** C++ Server -> React UI (all connected clients)
    *   **Purpose:** Notifies clients that a new user has connected.
    *   **Payload Example:** `{"type": "server_client_connected", "payload": {"user_id": "sess_yyyy", "message": "A new user has connected.", "timestamp": "2023-10-27T10:31:00Z"}}`

*   **`server_client_disconnected`**
    *   **Direction:** C++ Server -> React UI (all connected clients)
    *   **Purpose:** Notifies clients that a user has disconnected.
    *   **Payload Example:** `{"type": "server_client_disconnected", "payload": {"user_id": "sess_zzzz", "message": "A user has disconnected.", "timestamp": "2023-10-27T10:32:00Z"}}`

## C++ WebSocket Server

### Requirements
*   C++11 (or newer) compatible compiler (e.g., GCC, Clang, MSVC)
*   CMake (version 3.0.0 or newer)
*   Boost libraries (version 1.71.0 or compatible, tested with 1.83.0), specifically:
    *   Boost.Asio
    *   Boost.Beast
    *   Boost.System
    *   Boost.JSON (if not header-only with your Boost version, ensure `libboost-json-dev` or equivalent is installed)
*   `build-essential` (or equivalent for compiling C++ projects)

### Building
1.  **Clone the repository (if applicable).**
2.  **Ensure Dependencies are Met:**
    On Debian/Ubuntu, you can install necessary packages with:
    ```bash
    sudo apt-get update
    sudo apt-get install -y build-essential cmake libboost-dev libboost-system-dev libboost-thread-dev libboost-json-dev
    ```
3.  **Build the project using CMake:**
    ```bash
    mkdir build
    cd build
    cmake ..
    make websocket-chat-server
    ```
    This will create an executable named `websocket-chat-server` in the `build` directory.

### Running
1.  Navigate to the `build` directory: `cd build`
2.  Execute the server:
    ```bash
    ./websocket-chat-server <port> [<num_threads>]
    ```
    -   `<port>`: The port number for the server to listen on (e.g., 8080).
    -   `[<num_threads>]`: Optional. Number of threads for the server's I/O context (defaults to 1).
    -   Example: `./websocket-chat-server 8080`

## React UI

### Requirements
*   Node.js (e.g., v16.x, v18.x or later)
*   npm (usually comes with Node.js)

### Setup
1.  Navigate to the React UI directory:
    ```bash
    cd react-chat-ui
    ```
2.  Install dependencies:
    ```bash
    npm install
    ```

### Running
1.  Ensure the C++ WebSocket server is running.
2.  In the `react-chat-ui` directory, run:
    ```bash
    npm start
    ```
3.  This will start the React development server and typically open the application automatically in your default web browser at `http://localhost:3000`. If it doesn't open automatically, navigate to that URL.

## Code Formatting (C++)

This project uses `clang-format` for consistent C++ code style. A `.clang-format` configuration file (based on Google's C++ style) is included in the root directory.

### Manual Formatting
To format C++ code manually, ensure `clang-format` is installed, then run it on the desired files:
```bash
clang-format -i src/main.cpp src/ChatServer.cpp src/Session.cpp # Or other .cpp/.hpp files
```

### Automatic Formatting (Pre-commit Hook)
A helper script is provided at `scripts/format-staged.sh` to automatically format staged C++ files before each commit.

**Setup:**
1.  Ensure `clang-format` is installed.
2.  From the project root, enable the pre-commit hook:
    ```bash
    # Ensure script is executable (if needed)
    # chmod +x scripts/format-staged.sh
    ln -s ../../scripts/format-staged.sh .git/hooks/pre-commit
    ```
    If you already have a pre-commit hook, integrate the script's functionality into your existing hook.

## Troubleshooting
*   **Server Connection Issues (React UI):**
    *   Ensure the C++ WebSocket server is running and listening on the correct port (default `ws://127.0.0.1:8080` for the UI).
    *   Check browser console for WebSocket connection errors.
    *   Verify firewall settings are not blocking the connection.
*   **C++ Build Errors:**
    *   "Boost headers not found": Confirm Boost development libraries (including specific components like json-dev) are installed and discoverable by CMake.
*   **React UI `npm start` fails:**
    *   Ensure Node.js and npm are correctly installed.
    *   Try removing `react-chat-ui/node_modules` and `react-chat-ui/package-lock.json` then run `npm install` again.

## Contributing
Contributions are welcome! Please open an issue to discuss significant changes or submit a pull request for bug fixes and improvements.

## License
This project is currently unlicensed. (Consider adding an OSI approved license like MIT if you plan to share it widely).
