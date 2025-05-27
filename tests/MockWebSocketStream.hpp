#ifndef MOCKWEBSOCKETSTREAM_HPP
#define MOCKWEBSOCKETSTREAM_HPP

#include "gmock/gmock.h"
#include "IWebSocketStream.hpp" // Path relative to include_directories (src and tests)

// Need to ensure beast::error_code and other types are fully defined for GMock
// These are already included in IWebSocketStream.hpp, but good to be mindful
// #include <boost/beast/core/flat_buffer.hpp>
// #include <boost/system/error_code.hpp>
// #include <boost/beast/websocket/rfc6455.hpp>
// #include <boost/asio/ip/tcp.hpp>
// #include <boost/asio/buffer.hpp>

// Forward declarations from IWebSocketStream.hpp are not sufficient for MOCK_METHOD arguments.
// Full definitions are required, which IWebSocketStream.hpp should provide.

class MockWebSocketStream : public IWebSocketStream {
public:
    // Using MOCK_CONST_METHOD for const methods
    MOCK_METHOD(bool, is_open, (), (const, override));
    
    // For methods taking reference parameters for error_code, ensure they are non-const
    MOCK_METHOD(void, connect, (const net::ip::tcp::endpoint& ep, beast::error_code& ec), (override));
    MOCK_METHOD(void, handshake, (const std::string& host, const std::string& target, beast::error_code& ec), (override));
    MOCK_METHOD(void, write, (const net::const_buffer& buffer, beast::error_code& ec), (override));
    MOCK_METHOD(std::size_t, read, (beast::flat_buffer& buffer, beast::error_code& ec), (override));
    MOCK_METHOD(void, close, (beast::websocket::close_code code, beast::error_code& ec), (override));
};

#endif // MOCKWEBSOCKETSTREAM_HPP
