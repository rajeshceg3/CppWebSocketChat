#include "BoostWebSocketStream.hpp"
#include <boost/asio/connect.hpp> // For net::connect in the connect method if needed by underlying socket
#include <boost/beast/http.hpp> // For http::field if setting user_agent in handshake decorator

// Constructor
BoostWebSocketStream::BoostWebSocketStream(net::io_context& ioc)
    : ws_(ioc) {}

// IWebSocketStream interface implementation
bool BoostWebSocketStream::is_open() const {
    return ws_.is_open();
}

void BoostWebSocketStream::handshake(const std::string& host, const std::string& target, beast::error_code& ec) {
    // Standard client handshake
    // It's assumed that the underlying TCP socket is already connected.
    // Set some common options (optional, but good practice)
    ws_.set_option(beast::websocket::stream_base::decorator(
        [](beast::websocket::request_type& req) {
            req.set(beast::http::field::user_agent,
                std::string(BOOST_BEAST_VERSION_STRING) +
                    " websocket-client-BoostWebSocketStream");
            // Add other headers if needed, e.g., compression options
            // req.set(beast::http::field::sec_websocket_extensions, "permessage-deflate");
        }
    ));
    ws_.handshake(host, target, ec);
}

void BoostWebSocketStream::connect(const tcp::endpoint& ep, beast::error_code& ec) {
    // This connects the underlying TCP socket.
    ws_.next_layer().connect(ep, ec);
}

void BoostWebSocketStream::write(const net::const_buffer& buffer, beast::error_code& ec) {
    ws_.write(buffer, ec);
}

std::size_t BoostWebSocketStream::read(beast::flat_buffer& buffer, beast::error_code& ec) {
    return ws_.read(buffer, ec);
}

void BoostWebSocketStream::close(beast::websocket::close_code code, beast::error_code& ec) {
    ws_.close(code, ec);
}
