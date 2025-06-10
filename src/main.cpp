// #include "ChatClient.hpp" // Commented out old client
#include "ChatServer.hpp"
#include <iostream>
#include <string>
#include <vector> // For thread list
#include <thread> // For std::thread
#include <algorithm> // for std::max

// Import namespaces for convenience
namespace net = boost::asio;
using tcp = net::ip::tcp;

// Main function to run the chat server
int main(int argc, char* argv[]) {
    try {
        // Check command line arguments.
        if (argc < 2) {
            std::cerr << "Usage: websocket-chat-server <port> [<num_threads>]\n";
            return 1;
        }

        auto const address = net::ip::make_address("0.0.0.0");
        auto const port = static_cast<unsigned short>(std::atoi(argv[1]));
        int num_threads = 1;
        if (argc == 3) {
            num_threads = std::max<int>(1, std::atoi(argv[2]));
        }


        // The io_context is required for all I/O
        net::io_context ioc{num_threads};

        // Create and launch a listening port
        // ChatServer needs to be managed by shared_ptr if its methods (like on_accept creating Session)
        // rely on shared_from_this patterns indirectly, or if Sessions need to keep ChatServer alive.
        // For now, ChatServer itself doesn't use enable_shared_from_this, but Sessions it creates do.
        // Storing it as a shared_ptr is safer for lifetime management with async operations.
        auto server = std::make_shared<ChatServer>(ioc, tcp::endpoint{address, port});
        server->run(); // This typically calls do_accept()

        std::cout << "WebSocket Chat Server started on address " << address.to_string()
                  << " port " << port << " with " << num_threads << " thread(s)." << std::endl;

        // Run the I/O service on the requested number of threads
        std::vector<std::thread> v;
        v.reserve(num_threads > 0 ? num_threads -1 : 0); // Ensure num_threads-1 is not negative
        for(auto i = num_threads - 1; i > 0; --i) { // Only create threads if num_threads > 1
            v.emplace_back(
                [&ioc] {
                    try {
                        ioc.run();
                    } catch (const std::exception& e_thread) {
                        std::cerr << "Exception in worker thread: " << e_thread.what() << std::endl;
                    }
                });
        }

        // Main thread also runs ioc.run() if num_threads >= 1
        if (num_threads > 0) {
            ioc.run();
        } else { // Should not happen with std::max(1, ...) but as a safeguard
            std::cerr << "Error: Number of threads must be at least 1." << std::endl;
            return 1;
        }


        // If ioc.run() returns, it means all work is done or ioc.stop() was called.
        // For a server, this usually means it was stopped.
        std::cout << "Server io_context has stopped." << std::endl;

        // Block until all threads exit
        for(auto& t : v) {
            if (t.joinable()) {
                t.join();
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Exception in main: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown unhandled exception in main." << std::endl;
        return 1; // Indicate failure
    }
    std::cout << "Server shutting down." << std::endl;
    return 0;
}