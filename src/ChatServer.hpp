// ChatServer.hpp
#ifndef CHAT_SERVER_HPP
#define CHAT_SERVER_HPP

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <set>
#include <memory>
#include <string>

// Forward declaration
class Session;

namespace net = boost::asio;
namespace beast = boost::beast;
using tcp = net::ip::tcp;

class ChatServer {
public:
    ChatServer(net::io_context& ioc, const tcp::endpoint& endpoint);

    void run();
    // Overload broadcast: one for system messages, one for user messages that require sender info
    void broadcast(const std::string& message); // For system messages (no specific sender)
    void broadcast(const std::string& message, std::shared_ptr<Session> sender_session); // For user messages
    void on_client_connect(std::shared_ptr<Session> session);
    void on_client_disconnect(std::shared_ptr<Session> session);

private:
    void do_accept();
    void on_accept(beast::error_code ec, tcp::socket socket);

    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    std::set<std::shared_ptr<Session>> sessions_;
};

#endif // CHAT_SERVER_HPP
