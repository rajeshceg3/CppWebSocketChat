// Session.cpp
#include "Session.hpp"
#include "ChatServer.hpp" // Required for server_.broadcast and on_client_disconnect
#include "Utils.hpp"      // For getCurrentTimestampISO8601
#include <iostream>
#include <boost/json.hpp> // For Boost.JSON
#include <boost/uuid/uuid.hpp>            // For UUID generation if chosen
#include <boost/uuid/uuid_generators.hpp> // For UUID generation
#include <boost/uuid/uuid_io.hpp>         // For UUID string conversion
#include <sstream>      // For string stream, alternative to UUID for simpler ID
#include <iomanip>      // For std::hex, std::setw, std::setfill
#include <random>       // For random number generation for ID


// Static member initialization
std::atomic<int> Session::s_id_counter_(0);
namespace http = beast::http; // Add http namespace alias
namespace json = boost::json; // Add json namespace alias

// Helper to generate a unique session ID (simple counter for now)
std::string Session::generate_session_id() {
    // Simple counter based ID
    // return "session_" + std::to_string(++s_id_counter_);

    // More robust / unique ID using random numbers
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < 8; ++i) { // Generate a 16-character hex ID
        ss << std::setw(2) << distrib(gen);
    }
    return "sess_" + ss.str();
}


Session::Session(net::io_context& ioc, tcp::socket&& socket, ChatServer& server)
    : ws_(std::move(socket)), server_(server), strand_(net::make_strand(ioc.get_executor())) { // Initialized with ioc
    session_id_ = generate_session_id();
    nickname_ = "User" + session_id_; // Initialize nickname
    std::cout << "Session created with ID: " << session_id_ << " and Nickname: " << nickname_ << std::endl;
}

std::string Session::get_id() const {
    return session_id_;
}

void Session::set_nickname(const std::string& new_nickname) {
    nickname_ = new_nickname;
    // Optionally, log nickname changes or notify other systems/users.
    std::cout << "Session " << session_id_ << " nickname changed to: " << nickname_ << std::endl;
}

std::string Session::get_nickname() const {
    return nickname_;
}

void Session::run() {
    // We need to be executing within a strand to perform async operations
    // on the websocket stream.
    net::dispatch(ws_.get_executor(),
        beast::bind_front_handler(
            &Session::on_run,
            shared_from_this()));
}

// Moved the initial handshake to on_run to ensure it's on the strand
void Session::on_run() {
    // Set suggested timeout settings for the websocket
    ws_.set_option(
        websocket::stream_base::timeout::suggested(
            beast::role_type::server));

    // Set a decorator to change the Server of the handshake
    ws_.set_option(websocket::stream_base::decorator(
        [](websocket::response_type& res) {
            res.set(http::field::server,
                std::string(BOOST_BEAST_VERSION_STRING) +
                    " websocket-chat-server-cpp");
        }));

    // Accept the websocket handshake
    ws_.async_accept(
        beast::bind_front_handler(
            &Session::on_accept,
            shared_from_this()));
}


void Session::on_accept(beast::error_code ec) {
    if (ec) {
        std::cerr << "Session " << session_id_ << " Accept error: " << ec.message() << std::endl;
        server_.on_client_disconnect(shared_from_this()); // Notify server
        return;
    }
    std::cout << "Session " << session_id_ << " WebSocket handshake accepted." << std::endl;

    // The server_client_connected message is now sent by ChatServer::on_client_connect,
    // which has access to the session's nickname.
    // server_.on_client_connect(shared_from_this()); // This is already called from ChatServer::on_accept

    // Start reading messages
    do_read();
}

void Session::do_read() {
    // Read a message into our buffer
    ws_.async_read(
        buffer_,
        // Ensure this handler is dispatched on the strand
        net::bind_executor(strand_,
            beast::bind_front_handler(
                &Session::on_read,
                shared_from_this())));
}

void Session::on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    // This indicates that the session was closed
    if (ec == websocket::error::closed || ec == beast::http::error::end_of_stream) { // Fully qualified http error
        std::cout << "Session " << session_id_ << " closed by client." << std::endl;
        on_close(ec);
        server_.on_client_disconnect(shared_from_this()); // Notify server
        return;
    }

    if (ec) {
        std::cerr << "Session " << session_id_ << " Read error: " << ec.message() << std::endl;
        // If an error occurs, consider closing the connection
        on_close(ec); // Attempt to close WebSocket gracefully (logs error)
        server_.on_client_disconnect(shared_from_this()); // Notify server
        return;
    }

    // For now, assume the message is text. Later, parse JSON.
    std::string received_msg = beast::buffers_to_string(buffer_.data());
    std::cout << "Session " << session_id_ << " Received: " << received_msg << std::endl;

    // Broadcast the message (or handle as per protocol)
    std::string received_msg_str = beast::buffers_to_string(buffer_.data());
    buffer_.consume(buffer_.size()); // Clear the buffer early

    // std::cout << "Session " << session_id_ << " Received raw: " << received_msg_str << std::endl;

    json::value received_json;
    try {
        received_json = json::parse(received_msg_str);
    } catch (const std::exception& e) {
        std::cerr << "Session " << session_id_ << " JSON parse error: " << e.what() << " from message: " << received_msg_str << std::endl;
        // Optionally, send an error message back to the client or close session
        // For now, just ignore malformed JSON and continue reading
        do_read();
        return;
    }

    if (!received_json.is_object()) {
        std::cerr << "Session " << session_id_ << " Received JSON is not an object: " << received_msg_str << std::endl;
        do_read();
        return;
    }

    const json::object& msg_obj = received_json.as_object();
    if (!msg_obj.contains("type") || !msg_obj.at("type").is_string()) {
        std::cerr << "Session " << session_id_ << " Received JSON has no/invalid 'type': " << received_msg_str << std::endl;
        do_read();
        return;
    }

    std::string msg_type = msg_obj.at("type").as_string().c_str();

    if (msg_type == "client_send_message") {
        if (!msg_obj.contains("payload") || !msg_obj.at("payload").is_object()) {
            std::cerr << "Session " << session_id_ << " 'client_send_message' has no/invalid 'payload': " << received_msg_str << std::endl;
            do_read();
            return;
        }
        const json::object& payload_obj = msg_obj.at("payload").as_object();
        if (!payload_obj.contains("text") || !payload_obj.at("text").is_string()) {
            std::cerr << "Session " << session_id_ << " 'client_send_message' payload has no/invalid 'text': " << received_msg_str << std::endl;
            do_read();
            return;
        }
        std::string text_content = payload_obj.at("text").as_string().c_str();

        // server_.broadcast is handled by ChatServer, which will now fetch nickname
        // Construct the message payload without nickname, ChatServer::broadcast will add it.
        json::object broadcast_json_obj = {
            {"type", "server_broadcast_message"},
            {"payload", {
                {"user_id", session_id_}, // User ID is still essential
                // Nickname will be added by ChatServer::broadcast
                {"text", text_content},
                {"timestamp", Utils::getCurrentTimestampISO8601()}
            }}
        };
        // Pass the shared_ptr<Session> to broadcast
        server_.broadcast(json::serialize(broadcast_json_obj), shared_from_this());


    } else if (msg_type == "client_set_nickname") {
        if (!msg_obj.contains("payload") || !msg_obj.at("payload").is_object()) {
            std::cerr << "Session " << session_id_ << " 'client_set_nickname' has no/invalid 'payload': " << received_msg_str << std::endl;
            do_read();
            return;
        }
        const json::object& payload_obj = msg_obj.at("payload").as_object();
        if (!payload_obj.contains("nickname") || !payload_obj.at("nickname").is_string()) {
            std::cerr << "Session " << session_id_ << " 'client_set_nickname' payload has no/invalid 'nickname': " << received_msg_str << std::endl;
            do_read();
            return;
        }
        std::string new_nickname = payload_obj.at("nickname").as_string().c_str();
        set_nickname(new_nickname); // Update the nickname

        // For now, don't broadcast nickname change to simplify.
        // Later, might send a "server_nickname_changed" message.
        // Example:
        // json::object ack_msg = {
        //     {"type", "server_nickname_ack"},
        //     {"payload", {
        //         {"user_id", session_id_},
        //         {"nickname", nickname_},
        //         {"message", "Nickname updated successfully."}
        //     }}
        // };
        // send(std::make_shared<std::string>(json::serialize(ack_msg)));


    } else {
        std::cerr << "Session " << session_id_ << " Unknown message type: " << msg_type << std::endl;
        // Optionally send an error or ignore
    }

    // Continue reading for next message
    do_read();
}

void Session::send(std::shared_ptr<const std::string> ss) {
    // Post our work to the strand, this ensures that messages are sent in order
    net::post(
        strand_,
        beast::bind_front_handler(
            &Session::on_send,
            shared_from_this(),
            ss));
}

// This function is called on the strand
void Session::on_send(std::shared_ptr<const std::string> ss) {
    // Always add to queue
    write_queue_.push_back(ss);

    // Are we already writing?
    if (write_queue_.size() > 1) {
        return; // We are already writing, just enqueued
    }

    // We are not currently writing, so send this message immediately
    do_write();
}


void Session::do_write() {
    if (write_queue_.empty()) {
        return;
    }

    // Check if WebSocket is open before writing
    if (!ws_.is_open()) {
        std::cerr << "Session " << session_id_ << " WebSocket is not open. Cannot write." << std::endl;
        write_queue_.clear(); // Clear queue as we can't send
        return;
    }

    // Get the message from the queue
    auto msg = write_queue_.front();

    // Send the message
    ws_.text(true); // Assuming text messages
    ws_.async_write(
        net::buffer(*msg),
        // Ensure this handler is dispatched on the strand
        net::bind_executor(strand_,
            beast::bind_front_handler(
                &Session::on_write,
                shared_from_this())));
}

void Session::on_write(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec) {
        std::cerr << "Session " << session_id_ << " Write error: " << ec.message() << std::endl;
        // server_.on_client_disconnect(shared_from_this()); // Notify server on write error
        // on_close(ec); // Attempt to close WebSocket gracefully
        return;
    }

    // Remove the message from the queue
    if (!write_queue_.empty()) {
        write_queue_.erase(write_queue_.begin());
    }

    // If there are more messages, send the next one
    if (!write_queue_.empty()) {
        do_write();
    }
}

void Session::on_close(beast::error_code ec) {
    // Handle WebSocket closure.
    // This is called when the read operation detects a close from the client,
    // or if we decide to close the session due to an error.
    if (ec && ec != websocket::error::closed && ec != beast::http::error::end_of_stream) { // Fully qualified http error
        std::cerr << "Session " << session_id_ << " WebSocket closed with error: " << ec.message() << std::endl;
    } else {
        std::cout << "Session " << session_id_ << " WebSocket closed." << std::endl;
    }
    // No need to call server_.on_client_disconnect here as it's called by the reader/acceptor usually
}
