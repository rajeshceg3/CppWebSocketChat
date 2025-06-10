// Session.hpp
#ifndef SESSION_HPP
#define SESSION_HPP

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <string>
#include <vector>
#include <queue> // For message queue

// Forward declaration
class ChatServer;

namespace net = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = net::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
public:
    // Constructor now takes io_context&
    Session(net::io_context& ioc, tcp::socket&& socket, ChatServer& server);

    void run();
    void send(std::shared_ptr<const std::string> ss);
    std::string get_id() const; // Added get_id() method

private:
    void on_accept(beast::error_code ec);
    void do_read();
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    void do_write();
    void on_write(beast::error_code ec, std::size_t bytes_transferred);
    void on_close(beast::error_code ec); // Not strictly in design but good for handling closure
    void on_run(); // Added declaration
    void on_send(std::shared_ptr<const std::string> ss); // Added declaration

    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;
    ChatServer& server_; // Reference to ChatServer for broadcasting
    std::vector<std::shared_ptr<const std::string>> write_queue_;
    std::string session_id_; // For identifying sessions

    // Strand to ensure sequential execution of handlers for this session
    net::strand<net::io_context::executor_type> strand_; // Reverted to io_context::executor_type

    // Helper to generate a simple unique ID
    static std::atomic<int> s_id_counter_;
    static std::string generate_session_id();
};

#endif // SESSION_HPP
