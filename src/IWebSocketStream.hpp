#ifndef IWEBSOCKETSTREAM_HPP
#define IWEBSOCKETSTREAM_HPP

#include <string>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/system/error_code.hpp>
#include <boost/beast/websocket/rfc6455.hpp> // For close_code
#include <boost/asio/ip/tcp.hpp> // For tcp::endpoint
#include <boost/asio/buffer.hpp> // For const_buffer

// Forward declare to reduce header coupling if possible, though endpoint might be too complex.
// namespace boost { namespace asio { namespace ip { class tcp; } } }
// namespace boost { namespace asio { class const_buffer; } }
// namespace boost { namespace beast { class flat_buffer; namespace websocket { enum class close_code; } } }
// namespace boost { namespace system { class error_code; } }


namespace beast = boost::beast;
namespace net = boost::asio; // Added for consistency

class IWebSocketStream {
public:
    virtual ~IWebSocketStream() = default;

    // Methods based on Boost.Beast.WebSocket.Stream
    virtual bool is_open() const = 0;
    
    // Simplified connect for the interface - original takes resolver results
    // For now, let's assume the client will resolve and pass an endpoint.
    // This might need adjustment when BoostWebSocketStream is implemented.
    virtual void connect(const net::ip::tcp::endpoint& ep, beast::error_code& ec) = 0;
    
    virtual void handshake(const std::string& host, const std::string& target, beast::error_code& ec) = 0;
    
    // Using net::const_buffer for write, consistent with Boost.Asio
    virtual void write(const net::const_buffer& buffer, beast::error_code& ec) = 0;
    
    // Using beast::flat_buffer for read, consistent with Boost.Beast
    virtual std::size_t read(beast::flat_buffer& buffer, beast::error_code& ec) = 0;
    
    virtual void close(beast::websocket::close_code code, beast::error_code& ec) = 0;

    // Add next_layer() equivalent if needed for direct socket options,
    // but try to avoid exposing underlying layers in the interface if possible.
    // virtual tcp::socket& next_layer() = 0; // Example, might not be directly mockable or desirable

    // Add set_option if necessary, though this can be complex to abstract
    // virtual void set_option( /* ... */ ) = 0;
};

#endif // IWEBSOCKETSTREAM_HPP
