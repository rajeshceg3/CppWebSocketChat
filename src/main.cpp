#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/format.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

// Chat client implementation using websocket and boost asio
class ChatClient {
public:
    ChatClient(std::string const& host, std::string const& port)
        : io_context(1),
          ws(io_context, websocket::stream_base::version::latest)
    {
        // Resolve the host and port
        auto const address = net::ip::make_address(host);
        auto const endpoint = tcp::endpoint(address, std::stoi(port));

        // Connect to the server
        connect(ws.next_layer(), endpoint);

        // Set up the websocket connection
        ws.set_option(websocket::stream_base::decorator(
            websocket::stream_base::decorator::token_compression(websocket::stream_base::decorator::token_compression::enabled)
        ));
        ws.handshake(host, "/");
    }

    // Send a message to the chat server
    void send_message(const std::string& message) {
        ws.write(net::buffer(message));
    }

    // Receive a message from the chat server
    std::string receive_message() {
        beast::flat_buffer buffer;
        ws.read(buffer);
        return boost::beast::buffers_to_string(buffer.data());
    }

    // Run the client loop
    void run() {
        while (true) {
            std::cout << "> ";
            std::string message;
            std::getline(std::cin, message);

            if (message == "/quit") {
                break;
            }

            send_message(message);
            std::cout << "Server: " << receive_message() << std::endl;
        }

        // Close the websocket connection
        ws.close(websocket::close_code::normal);
    }

private:
    net::io_context io_context;
    websocket::stream<tcp::socket> ws;
};

int main() {
    std::string host = "localhost";
    std::string port = "8080"; // Replace with your server's port

    ChatClient client(host, port);

    std::cout << "Connected to chat server!" << std::endl;
    client.run();

    return 0;
}