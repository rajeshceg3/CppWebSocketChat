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

// Broadcast for system messages (no specific sender context for nickname)
void ChatServer::broadcast(const std::string& message) {
    auto const shared_message = std::make_shared<const std::string>(message);
    for (auto session_ptr : sessions_) {
        session_ptr->send(shared_message);
    }
}

// Broadcast for messages from a specific client, adding their nickname
void ChatServer::broadcast(const std::string& message_json_str, std::shared_ptr<Session> sender_session) {
    if (!sender_session) return;

    std::string final_message_str;
    try {
        json::value parsed_message = json::parse(message_json_str);
        json::object& msg_obj = parsed_message.as_object(); // Assuming root is an object

        if (msg_obj.contains("type") && msg_obj.at("type").as_string() == "server_broadcast_message") {
            if (msg_obj.contains("payload") && msg_obj.at("payload").is_object()) {
                json::object& payload_obj = msg_obj.at("payload").as_object();
                payload_obj["nickname"] = sender_session->get_nickname(); // Add nickname
            }
        }
        // For other message types originating from a client, if any, we might add nickname too,
        // but for now, explicitly handling server_broadcast_message.
        final_message_str = json::serialize(parsed_message);
    } catch (const std::exception& e) {
        std::cerr << "Error modifying message for broadcast: " << e.what() << ". Original message: " << message_json_str << std::endl;
        final_message_str = message_json_str; // Send original if modification fails
    }

    auto const shared_final_message = std::make_shared<const std::string>(final_message_str);
    for (auto session_ptr : sessions_) {
        // Send to all sessions, including the sender, so sender also sees their nickname.
        // If sender should be excluded for some messages, the calling context (e.g., Session::on_read)
        // would need to use the system broadcast or handle it.
        session_ptr->send(shared_final_message);
    }
}


void ChatServer::on_client_connect(std::shared_ptr<Session> session) {
    sessions_.insert(session);
    std::cout << "Client '" << session->get_id() << "' (Nick: '" << session->get_nickname() << "') added to active sessions. Total clients: " << sessions_.size() << std::endl;

    // This message is now sent from Session::on_accept using its own session_id and initial nickname
    // If we want ChatServer to broadcast this, it would look like:
    json::object connected_json_obj = {
        {"type", "server_client_connected"},
        {"payload", {
            {"user_id", session->get_id()},
            {"nickname", session->get_nickname()}, // Add nickname here
            {"message", "User has connected."},
            {"timestamp", Utils::getCurrentTimestampISO8601()}
        }}
    };
    // This broadcast goes to ALL clients, including the new one.
    // The message is constructed here, so it uses the system broadcast.
    broadcast(json::serialize(connected_json_obj));
}

void ChatServer::on_client_disconnect(std::shared_ptr<Session> session) {
    std::string session_id = session->get_id();
    std::string nickname = session->get_nickname(); // Get nickname before session is invalidated
    sessions_.erase(session);
    std::cout << "Client disconnected: " << session_id << " (Nick: '" << nickname << "'). Total clients: " << sessions_.size() << std::endl;

    json::object disconnected_json = {
        {"type", "server_client_disconnected"},
        {"payload", {
            {"user_id", session_id},
            {"nickname", nickname}, // Add nickname here
            {"message", "User has disconnected."},
            {"timestamp", Utils::getCurrentTimestampISO8601()}
        }}
    };
    broadcast(json::serialize(disconnected_json)); // Use system broadcast
}
