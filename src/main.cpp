#include "ChatClient.hpp" // Include the new header
#include <iostream>
#include <string>
// Other necessary includes for main.cpp, if any, but most should be in ChatClient.hpp/cpp

int main(int argc, char* argv[]) {
    std::string host = "127.0.0.1"; // Default host
    std::string port = "8080";     // Default port

    if (argc == 3) {
        host = argv[1];
        port = argv[2];
    } else if (argc == 1) {
        // Use default host and port
    } else {
        std::cerr << "Usage: " << argv[0] << " [host port]" << std::endl;
        return 1;
    }

    std::cout << ChatClient::getCurrentTimestamp() << " Attempting to connect to " << host << ":" << port << "..." << std::endl;

    try {
        ChatClient client(host, port);

        if (client.is_connected()) {
            std::cout << ChatClient::getCurrentTimestamp() << " Connected to " << host << ":" << port << "." << std::endl;
            // The run loop now prints "Chat client started..." or similar
            client.run(std::cin, std::cout); // Pass std::cin and std::cout
        } else {
            std::cout << ChatClient::getCurrentTimestamp() << " Failed to connect to the chat server at " << host << ":" << port << ". Exiting." << std::endl;
            return 1; // Indicate failure
        }

    } catch (std::exception const& e) {
        std::cerr << ChatClient::getCurrentTimestamp() << " Unhandled exception in main: " << e.what() << std::endl;
        return 1; // Indicate failure
    } catch (...) {
        std::cerr << "Unknown unhandled exception in main." << std::endl;
        return 1; // Indicate failure
    }

    return 0;
}