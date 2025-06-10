// ChatServer.cpp
#include "ChatServer.hpp"
#include "Session.hpp"
#include "Utils.hpp"      // For getCurrentTimestampISO8601
#include <iostream>
#include <boost/json.hpp> // For Boost.JSON

namespace json = boost::json; // Add json namespace alias

ChatServer::ChatServer(net::io_context& ioc, const tcp::endpoint& endpoint)
    : ioc_(ioc), acceptor_(ioc) {
    beast::error_code ec;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
        std::cerr << "Failed to open acceptor: " << ec.message() << std::endl;
        return;
    }

    // Allow address reuse
    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) {
        std::cerr << "Failed to set socket options: " << ec.message() << std::endl;
        return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if (ec) {
        std::cerr << "Failed to bind to address: " << ec.message() << std::endl;
        return;
    }

    // Start listening for connections
    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
        std::cerr << "Failed to listen on acceptor: " << ec.message() << std::endl;
        return;
    }
}

void ChatServer::run() {
    if (acceptor_.is_open()) {
        do_accept();
    }
}

void ChatServer::do_accept() {
    // The new connection gets its own strand
    acceptor_.async_accept(
        net::make_strand(ioc_),
        beast::bind_front_handler(
            &ChatServer::on_accept,
            this)); // Changed from shared_from_this() as ChatServer might not be a shared_ptr
}

void ChatServer::on_accept(beast::error_code ec, tcp::socket socket) {
    if (ec) {
        std::cerr << "Accept error: " << ec.message() << std::endl;
    } else {
        // Create the session and run it, passing ioc_
        auto new_session = std::make_shared<Session>(ioc_, std::move(socket), *this);
        on_client_connect(new_session); // Add to set
        new_session->run(); // Start the session
    }

    // Accept another connection
    do_accept();
}

void ChatServer::broadcast(const std::string& message, Session* exclude_session) {
    auto const shared_message = std::make_shared<const std::string>(message);
    for (auto session : sessions_) {
        if (session.get() != exclude_session) {
            session->send(shared_message);
        }
    }
}

void ChatServer::on_client_connect(std::shared_ptr<Session> session) {
    sessions_.insert(session);
    // Message is now sent from Session::on_accept
    std::cout << "Client '" << session->get_id() << "' added to active sessions. Total clients: " << sessions_.size() << std::endl;
}

void ChatServer::on_client_disconnect(std::shared_ptr<Session> session) {
    std::string session_id = session->get_id(); // Get ID before session is potentially invalidated further
    sessions_.erase(session);
    std::cout << "Client disconnected: " << session_id << ". Total clients: " << sessions_.size() << std::endl;

    json::value disconnected_json = {
        {"type", "server_client_disconnected"},
        {"payload", {
            {"user_id", session_id},
            {"message", "A user has disconnected."},
            {"timestamp", Utils::getCurrentTimestampISO8601()}
        }}
    };
    broadcast(json::serialize(disconnected_json));
}
