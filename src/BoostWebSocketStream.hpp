#ifndef BOOSTWEBSOCKETSTREAM_HPP
#define BOOSTWEBSOCKETSTREAM_HPP

#include "IWebSocketStream.hpp"
#include <boost/asio/io_context.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp> // Required for tcp::socket

// Forward declarations (already included in IWebSocketStream.hpp, but good practice)
// namespace boost { namespace beast { class flat_buffer; namespace websocket { enum class close_code; } } }
// namespace boost { namespace system { class error_code; } }
// namespace boost { namespace asio { class const_buffer; namespace ip { class tcp; } } }


namespace beast = boost::beast;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class BoostWebSocketStream : public IWebSocketStream {
public:
    BoostWebSocketStream(net::io_context& ioc);
    ~BoostWebSocketStream() override = default;

    // IWebSocketStream interface implementation
    bool is_open() const override;
    void handshake(const std::string& host, const std::string& target, beast::error_code& ec) override;
    void connect(const tcp::endpoint& ep, beast::error_code& ec) override; 
    void write(const net::const_buffer& buffer, beast::error_code& ec) override;
    std::size_t read(beast::flat_buffer& buffer, beast::error_code& ec) override;
    void close(beast::websocket::close_code code, beast::error_code& ec) override;

    // Helper to expose the underlying stream's next_layer for specific Boost operations if needed
    // This should be used sparingly and ideally not part of IWebSocketStream.
    // For example, setting socket options before connect.
    // tcp::socket& next_layer() { return ws_.next_layer(); } 
    // BoostWebSocketStream is concrete, so it can expose this.

private:
    websocket::stream<tcp::socket> ws_;
};

#endif // BOOSTWEBSOCKETSTREAM_HPP
