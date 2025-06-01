# C++ Boost WebSocket Chat Client

## Description
A simple command-line chat client using C++ and Boost (Asio/Beast) for WebSocket communication. It allows users to connect to a specified chat server, send messages, and receive messages in real-time.

## Features
- Connects to a WebSocket chat server.
- Sends messages entered by the user.
- Receives and displays messages from the server.
- Basic error handling for network operations.
- Cross-platform compatibility (where Boost and C++ compilers are available).

## Requirements
- C++11 (or newer) compatible compiler (e.g., GCC, Clang, MSVC)
- CMake (version 3.0.0 or newer)
- Boost libraries (version 1.83.0 or compatible), specifically:
  - Boost.Asio
  - Boost.Beast
  - Boost.System

## Installation & Building

1.  **Clone the repository (if applicable):**
    ```bash
    git clone <repository-url>
    cd <repository-directory>
    ```

2.  **Ensure Dependencies are Met:**
    Make sure you have a suitable C++ compiler, CMake, and the Boost libraries installed on your system.
    On Debian/Ubuntu, you can install Boost development libraries with:
    ```bash
    sudo apt-get update
    sudo apt-get install libboost-dev
    ```
    For other systems, please refer to the official Boost documentation for installation instructions.

3.  **Build the project using CMake:**
    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```
    This will create an executable named `main` (or `main.exe` on Windows) in the `build` directory.

## Usage
1.  **Run the client:**
    Navigate to the `build` directory and execute the client:
    ```bash
    ./main
    ```
    The client can be run with optional command-line arguments to specify the server host and port:
    ```bash
    ./main [host port]
    ```
    - If no arguments are provided, it defaults to connecting to `127.0.0.1` on port `8080`.
    - For example, to connect to a server at `mychatserver.com` on port `12345`:
      ```bash
      ./main mychatserver.com 12345
      ```
2.  **Interacting with the Chat:**
    - Once connected, type your message and press Enter to send.
    - Received messages will be displayed with a "Server:" prefix.
    - Type `/quit` and press Enter to disconnect and exit the client.

## Troubleshooting
- **Connection Errors:**
  - "Connection refused" or similar: Ensure the chat server is running and accessible at the configured host and port. Check firewall settings.
  - "Handshake failed": The server might not be a WebSocket server, or there's a protocol mismatch.
- **Build Errors:**
  - "Boost headers not found": Make sure Boost development libraries are correctly installed and discoverable by CMake. You might need to set `BOOST_ROOT` or other CMake variables.
  - "Compiler errors": Ensure your compiler supports C++11 or newer.

## Contributing
Contributions are welcome! If you find any issues or have suggestions for improvements, please open an issue or submit a pull request.

## License
This project is currently unlicensed. Please specify a license if distributing.

## Code Formatting

This project uses `clang-format` to ensure consistent code style. A `.clang-format` configuration file (based on Google's C++ style) is included in the root directory.

### Manual Formatting
To format your code manually, ensure `clang-format` is installed, then run it on the desired files:
```bash
clang-format -i src/main.cpp # Or other .cpp/.h files
```

### Automatic Formatting (Pre-commit Hook)
A helper script is provided to automatically format staged C++ files before each commit.

**Setup:**
1.  Ensure `clang-format` is installed and available in your system's PATH.
2.  Navigate to the root directory of the project.
3.  The script is located at `scripts/format-staged.sh`. It should already be executable. If not, run:
    ```bash
    chmod +x scripts/format-staged.sh
    ```
4.  To enable the pre-commit hook, create a symbolic link from `.git/hooks/pre-commit` to this script:
    ```bash
    ln -s ../../scripts/format-staged.sh .git/hooks/pre-commit
    ```
    If you already have a pre-commit hook, you'll need to integrate this script's functionality into your existing hook.

Now, `clang-format` will automatically format your staged `.cpp` and `.h` files each time you run `git commit`. If formatting changes are made, the commit will be paused, and you'll need to `git add` the changes again before proceeding with the commit.
