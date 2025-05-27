#include "ChatClient.hpp"
#include "BoostWebSocketStream.hpp" // Include the concrete stream implementation
#include <boost/asio/ip/tcp.hpp>   // For tcp::resolver
#include <boost/format.hpp> 
#include <iostream> 

// Constructor for production use
ChatClient::ChatClient(std::string const& host, std::string const& port)
    : io_context_(), 
      ioc_for_stream_(io_context_), 
      ws_impl_(std::make_unique<BoostWebSocketStream>(ioc_for_stream_)),
      connected_(false),
      host_(host), 
      port_(port) 
{
    log("ChatClient(host, port) constructor called. Attempting connection...");
    try {
        tcp::resolver resolver(ioc_for_stream_); // Use the same io_context as the stream
        auto const results = resolver.resolve(host_, port_);
        
        if (results.empty()) {
            log("DNS resolution failed for " + host_ + ":" + port_, true);
            return; // connected_ remains false
        }

        // Assuming results is not empty, connect to the first endpoint
        tcp::endpoint endpoint = results.begin()->endpoint();
        
        beast::error_code ec;
        ws_impl_->connect(endpoint, ec); // Connect the underlying TCP socket
        
        if (ec) {
            log("TCP Connect to " + host_ + ":" + port_ + " failed: " + ec.message(), true);
            return; // connected_ remains false
        }

        // Perform the WebSocket handshake
        ws_impl_->handshake(host_, "/", ec); // Using member host_ and target "/"
        if (ec) {
            log("WebSocket Handshake with " + host_ + ":" + port_ + " failed: " + ec.message(), true);
            return; // connected_ remains false
        }

        // If all successful
        connected_ = true;
        log("Successfully connected to server: " + host_ + ":" + port_);

    } catch (boost::system::system_error const& e) { // Catch boost system errors specifically
        log("Connection error (boost::system::system_error): " + std::string(e.what()), true);
        connected_ = false;
    } catch (std::exception const& e) { // Catch other standard exceptions
        log("Connection error (std::exception): " + std::string(e.what()), true);
        connected_ = false;
    }
}

// Constructor for testing and dependency injection
ChatClient::ChatClient(std::unique_ptr<IWebSocketStream> stream_impl, net::io_context& ioc_ref)
    : io_context_(), // This specific io_context_ might not be used if stream has its own.
      ioc_for_stream_(ioc_ref), 
      ws_impl_(std::move(stream_impl)),
      connected_(false), 
      host_("N/A (injected stream)"), 
      port_("N/A (injected stream)")
{
    log("ChatClient(stream_impl, ioc_ref) constructor called.");
    if (ws_impl_ && ws_impl_->is_open()) {
         connected_ = true; 
         log("Injected stream is already open.");
    } else if (ws_impl_) {
        log("Injected stream is not open. Connection/handshake may be needed externally.");
        // For a truly unit testable client, connect/handshake might be separate public methods
        // or the test setup would ensure the mock is in a desired state.
    } else {
        log("Injected stream is null.", true);
    }
}

// Destructor
ChatClient::~ChatClient() {
    log("ChatClient destructor called.");
    if (ws_impl_ && ws_impl_->is_open()) {
        try {
            log("Attempting to close WebSocket connection via ws_impl_.");
            beast::error_code ec; 
            ws_impl_->close(websocket::close_code::normal, ec); 
            if(ec) {
                log("Error code during close in destructor: " + ec.message(), true);
            } else {
                log("WebSocket connection closed successfully via ws_impl_ in destructor.");
            }
        } catch (boost::system::system_error const& e) {
            log("boost::system::system_error closing websocket in destructor: " + std::string(e.what()), true);
        } catch (std::exception const& e) {
            log("std::exception closing websocket in destructor: " + std::string(e.what()), true);
        }
    } else {
        log("Destructor: ws_impl_ is null or was not open.");
    }
}

void ChatClient::log(const std::string& message, bool is_error) {
    // Simple console logging; replace with a more robust logger if needed.
    // [CLIENT LOG] or [ERROR] prefix helps distinguish.
    // Consider adding timestamps or thread IDs for more complex scenarios.
    if (is_error) {
        std::cerr << "[ERROR] ChatClient: " << message << std::endl;
    } else {
        std::cout << "[CLIENT LOG] ChatClient: " << message << std::endl;
    }
}

bool ChatClient::is_connected() const {
    // `connected_` is the primary source of truth, managed by connect/handshake/error logic.
    // Additionally, ensure `ws_impl_` is not null.
    return connected_ && ws_impl_ != nullptr;
}

// Send a message to the chat server
bool ChatClient::send_message(const std::string& message) {
    if (!is_connected() || !ws_impl_) { // Added !ws_impl_ check
        log("Cannot send message: Not connected or stream not initialized.", true);
        return false;
    }
    try {
        beast::error_code ec;
        ws_impl_->write(net::buffer(message), ec);
        if (ec) {
            log("Error sending message (write): " + ec.message(), true);
            if (ec == websocket::error::closed || ec == beast::error::timeout || 
                ec == net::error::broken_pipe || ec == net::error::connection_reset) {
                log("Connection closed or broken while sending.", true);
                connected_ = false; 
            }
            return false;
        }
        return true;
    } catch (boost::system::system_error const& e) { // Should ideally be caught by ec from write
        log("Exception sending message: " + std::string(e.what()), true);
        connected_ = false; 
        return false;
    } catch (std::exception const& e) {
        log("Generic exception sending message: " + std::string(e.what()), true);
        connected_ = false; 
        return false;
    }
}

// Receive a message from the chat server
std::string ChatClient::receive_message() {
    if (!is_connected() || !ws_impl_) { // Added !ws_impl_ check
        log("Cannot receive message: Not connected or stream not initialized.", true);
        return "[Error: Not connected or connection lost]";
    }
    try {
        beast::flat_buffer buffer;
        beast::error_code ec;
        ws_impl_->read(buffer, ec);

        if (ec) {
            log("Error receiving message (read): " + ec.message(), true);
            if (ec == websocket::error::closed) {
                log("Connection closed by server.", true);
                connected_ = false;
                return "[Connection closed by server]";
            } else if (ec == beast::error::timeout || ec == net::error::broken_pipe ||
                       ec == net::error::connection_reset) { 
                log("Connection broken or timed out while receiving.", true);
                connected_ = false; 
                return "[Error: Connection broken or timed out]";
            }
            connected_ = false; 
            log("Marking as disconnected due to receive error.", true);
            return "[Error: Failed to read message]";
        }
        return boost::beast::buffers_to_string(buffer.data());
    } catch (boost::system::system_error const& e) { // Should ideally be caught by ec from read
        log("Exception receiving message: " + std::string(e.what()), true);
        connected_ = false; 
        return "[Error: Exception while reading message]";
    } catch (std::exception const& e) {
        log("Generic exception receiving message: " + std::string(e.what()), true);
        connected_ = false; 
        return "[Error: Generic failure to read message]";
    }
}

// Run the client loop (User interaction part)
void ChatClient::run(std::istream& input, std::ostream& output) {
    if (!is_connected()) {
        log("Client not connected. Cannot start run loop.", true);
        // output << "[ERROR] Client not connected. Cannot start run loop." << std::endl; // Optionally inform user via output
        return;
    }

    output << "Chat client started. Type '/quit' to exit." << std::endl;

    try {
        while (is_connected()) { 
            output << "> "; 
            std::string message;
            if (!std::getline(input, message)) { 
                log("Input stream ended or error.", !input.eof()); // Log as error if not EOF
                if (input.eof()) {
                    log("EOF detected on input stream.");
                }
                break; 
            }

            if (message == "/quit") {
                log("User initiated quit.");
                break;
            }
            
            if (message.empty()) { // Allow empty messages to be sent if desired, or handle here.
                continue;          // For now, skip empty lines.
            }

            // Check connection status again before sending
            // This is important if the loop can run for a while or if external factors affect connection.
            if (!is_connected()) { 
                log("Connection lost before sending message.", true);
                break;
            }
            
            if (!send_message(message)) {
                // send_message logs its own errors and updates connected_
                // log("Failed to send message.", true); // Already logged by send_message
                if(!is_connected()){ // If send_message caused disconnection
                    log("Connection lost after attempting to send message.", true);
                    break;
                }
                // If still connected, could be a non-fatal send error (though our current send_message makes most errors fatal)
                // Allow user to try again or quit.
                continue;
            }

            std::string server_response = receive_message(); 
            // receive_message logs its own errors and updates connected_

            // If receive_message caused disconnection and returned an error string
            if (!is_connected() && 
                (server_response.rfind("[Error:", 0) == 0 || server_response.rfind("[Connection closed", 0) == 0) ) {
                output << "Server: " << server_response << std::endl; // Show the error to the user
                log("Connection lost while receiving message or server closed connection.", true);
                break;
            } else if (!is_connected()) { // If disconnected for other reasons during receive
                 log("Connection lost during or after receive_message. No specific error message from receive_message.", true);
                 break;
            }
            
            output << "Server: " << server_response << std::endl;
        }
    } catch (std::exception const& e) { // Catch any unexpected exceptions
        log("Unhandled std::exception in run loop: " + std::string(e.what()), true);
        connected_ = false; // Ensure disconnected state
    }


    log("Exiting run loop.");
    if (ws_impl_ && ws_impl_->is_open()) {
        log("Closing WebSocket connection from run().");
        beast::error_code ec_close;
        ws_impl_->close(websocket::close_code::normal, ec_close);
        if (ec_close) {
            log("Error during WebSocket close: " + ec_close.message(), true);
        }
    }
    // Final message to user's output stream
    output << "Disconnected." << std::endl;
}
