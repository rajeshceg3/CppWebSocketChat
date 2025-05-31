#ifndef CHATCLIENT_HPP
#define CHATCLIENT_HPP

#include <iostream> 
#include <string>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/http.hpp> 
#include <boost/asio/connect.hpp> 
#include <boost/asio/ip/tcp.hpp>  
#include <memory> // For std::unique_ptr
#include "IWebSocketStream.hpp" // Include the new interface

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class ChatClient {
public:
    // Constructor for production use (will internally create BoostWebSocketStream later)
    ChatClient(std::string const& host, std::string const& port);
    // Constructor for testing and dependency injection
    ChatClient(std::unique_ptr<IWebSocketStream> stream_impl, net::io_context& ioc_ref); // Added ioc_ref
    ~ChatClient();

    bool send_message(const std::string& message);
    std::string receive_message();
    // void run(); // Old signature
    void run(std::istream& input, std::ostream& output); // New signature
    bool is_connected() const;

private:
    // io_context might be owned by ChatClient or passed in, depending on design.
    // If BoostWebSocketStream needs an io_context, and ChatClient creates it,
    // then io_context can remain a member. If stream is passed in, it might bring its own ioc.
    // For now, keep io_context as a member for the host/port constructor.
    // The DI constructor will require an io_context reference if the stream needs it.
    net::io_context io_context_; // Renamed to avoid conflict if a ref is also taken
    net::io_context& ioc_for_stream_; // Reference for streams that need an external io_context

    std::unique_ptr<IWebSocketStream> ws_impl_;
    bool connected_;
    std::string host_; 
    std::string port_;

    void log(const std::string& message, bool is_error = false);
};

#endif // CHATCLIENT_HPP
