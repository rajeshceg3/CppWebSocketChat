#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ChatServer.hpp"
#include "Session.hpp"
#include "Utils.hpp" // For timestamp, if we match it, though it's tricky
#include <boost/json.hpp>
#include <string>
#include <memory>
#include <vector> // To store captured messages

// Using declarations for GMock might be needed if we use more advanced GMock features
using ::testing::_;
using ::testing::NiceMock; // If we were mocking parts of Session construction

namespace json = boost::json;

// A derived Session class to capture messages sent to it
class CapturingSession : public Session {
public:
    // To call the base Session constructor, we need a valid tcp::socket.
    // For these tests, the socket won't actually be used for communication,
    // but it needs to be valid enough for Session construction.
    // We also need a ChatServer instance.
    CapturingSession(net::io_context& ioc, tcp::socket&& socket, ChatServer& server, const std::string& fixed_id = "")
        : Session(ioc, std::move(socket), server) {
        if (!fixed_id.empty()) {
            // Manually override session_id_ and default nickname_ if a fixed_id is given
            // This requires session_id_ and nickname_ to be protected or have setters accessible here.
            // For now, let's assume Session's constructor sets a usable ID and default nickname.
            // To make ID predictable for tests, we might need to adjust Session or use fixed IDs.
            // Let's try setting nickname directly after construction.
        }
    }

    // Override send to capture messages
    void send(std::shared_ptr<const std::string> ss) override {
        captured_messages.push_back(*ss);
    }

    std::vector<std::string> captured_messages;

    // Helper to get the base Session part as shared_ptr if needed by ChatServer
    std::shared_ptr<Session> shared_base_from_this() {
        return std::static_pointer_cast<Session>(shared_from_this());
    }
};


class ChatServerTest : public ::testing::Test {
protected:
    net::io_context ioc_;
    tcp::endpoint endpoint_{net::ip::make_address("127.0.0.1"), 8088}; // Test endpoint
    std::unique_ptr<ChatServer> server_;

    // Store capturing sessions to inspect their messages
    std::vector<std::shared_ptr<CapturingSession>> capturing_sessions_list_;

    void SetUp() override {
        server_ = std::make_unique<ChatServer>(ioc_, endpoint_);
        // ChatServer itself doesn't listen or accept in unit tests unless run() is called.
        // We will manually add sessions.
    }

    // Helper to create a CapturingSession and add it to the server
    // Returns a raw pointer for convenience in tests, but it's managed by shared_ptr in capturing_sessions_list_
    // and by ChatServer's internal sessions_ set.
    std::shared_ptr<CapturingSession> add_capturing_session_to_server(const std::string& nickname_to_set) {
        tcp::socket test_socket(ioc_);
        // Socket doesn't need to be connected for these tests as ws_.async_accept etc. are not called.
        // Session constructor might still fail if it requires an open socket for ws_ stream init.
        // Session's constructor: ws_(std::move(socket)). This should be fine with a default socket.

        auto cap_session = std::make_shared<CapturingSession>(ioc_, std::move(test_socket), *server_);
        cap_session->set_nickname(nickname_to_set); // Set the nickname for the test

        // Add to ChatServer's session list using on_client_connect
        // This also triggers the server_client_connected broadcast we want to test.
        server_->on_client_connect(cap_session); // Pass the derived shared_ptr

        capturing_sessions_list_.push_back(cap_session);
        return cap_session;
    }

    void TearDown() override {
        // server_->stop(); // If server had a stop method
        ioc_.stop(); // Stop io_context if it were running
        capturing_sessions_list_.clear();
    }

    // Utility to parse JSON and check common fields
    ::testing::AssertionResult assert_json_message_basics(const std::string& json_str,
                                                          const std::string& expected_type,
                                                          const std::string& expected_user_id,
                                                          const std::string& expected_nickname) {
        try {
            json::value jv = json::parse(json_str);
            const json::object& obj = jv.as_object();

            if (!obj.contains("type") || obj.at("type").as_string() != expected_type) {
                return ::testing::AssertionFailure() << "JSON type is missing or incorrect. Expected: " << expected_type << ", Got: " << (obj.contains("type") ? obj.at("type").as_string().c_str() : "null");
            }

            if (!obj.contains("payload")) return ::testing::AssertionFailure() << "JSON payload is missing.";
            const json::object& payload = obj.at("payload").as_object();

            if (!payload.contains("user_id") || payload.at("user_id").as_string() != expected_user_id) {
                return ::testing::AssertionFailure() << "Payload user_id is missing or incorrect. Expected: " << expected_user_id << ", Got: " << (payload.contains("user_id") ? payload.at("user_id").as_string().c_str() : "null");
            }
            if (!payload.contains("nickname") || payload.at("nickname").as_string() != expected_nickname) {
                return ::testing::AssertionFailure() << "Payload nickname is missing or incorrect. Expected: " << expected_nickname << ", Got: " << (payload.contains("nickname") ? payload.at("nickname").as_string().c_str() : "null");
            }
            if (!payload.contains("timestamp")) return ::testing::AssertionFailure() << "Payload timestamp is missing.";
            // Could also validate timestamp format if necessary, but usually presence is enough.

            return ::testing::AssertionSuccess();
        } catch (const std::exception& e) {
            return ::testing::AssertionFailure() << "JSON parsing failed: " << e.what() << "\nJSON string: " << json_str;
        }
    }
};

TEST_F(ChatServerTest, ClientConnectBroadcastsNickname) {
    std::string test_nickname = "Tester1";

    // Create observer first.
    capturing_sessions_list_.clear();
    auto obs_session = add_capturing_session_to_server("Observer");

    // Now connect the session we are testing
    auto conn_session = add_capturing_session_to_server(test_nickname);
    std::string conn_session_id = conn_session->get_id();

    // obs_session should have received two messages:
    // 1. Its own connection message.
    // 2. conn_session's connection message.
    ASSERT_GE(obs_session->captured_messages.size(), 2) << "Observer should have received its own and the new client's connection message.";
    std::string connect_msg_for_observer = obs_session->captured_messages[1]; // Second message

    EXPECT_TRUE(assert_json_message_basics(connect_msg_for_observer, "server_client_connected", conn_session_id, test_nickname));
    json::value jv_obs = json::parse(connect_msg_for_observer);
    EXPECT_EQ(jv_obs.as_object().at("payload").as_object().at("message").as_string(), "User has connected.");

    // conn_session should have received one message: its own connection.
    ASSERT_GE(conn_session->captured_messages.size(), 1);
    std::string connect_msg_for_self = conn_session->captured_messages[0];
    EXPECT_TRUE(assert_json_message_basics(connect_msg_for_self, "server_client_connected", conn_session_id, test_nickname));
}

TEST_F(ChatServerTest, ClientDisconnectBroadcastsNickname) {
    // 1. Add an observer session.
    auto observer_session = add_capturing_session_to_server("Observer");
    // 2. Add the session that will disconnect.
    std::string disconnecting_nickname = "Leaver";
    auto disconnecting_session = add_capturing_session_to_server(disconnecting_nickname);
    std::string disconnecting_id = disconnecting_session->get_id();

    // Clear observer's initial messages if we only care about the disconnect message.
    // (Two messages so far: its own connect, and disconnecting_session's connect)
    ASSERT_GE(observer_session->captured_messages.size(), 2);
    observer_session->captured_messages.erase(observer_session->captured_messages.begin(), observer_session->captured_messages.begin() + 2);


    // 3. Trigger disconnect.
    server_->on_client_disconnect(disconnecting_session);

    // 4. Check broadcast to observer_session.
    ASSERT_EQ(observer_session->captured_messages.size(), 1) << "Observer should have received the disconnect message.";
    std::string disconnect_msg = observer_session->captured_messages[0];

    EXPECT_TRUE(assert_json_message_basics(disconnect_msg, "server_client_disconnected", disconnecting_id, disconnecting_nickname));
    json::value jv = json::parse(disconnect_msg);
    EXPECT_EQ(jv.as_object().at("payload").as_object().at("message").as_string(), "User has disconnected.");
}

TEST_F(ChatServerTest, BroadcastMessageIncludesNickname) {
    // 1. Add an observer session.
    auto observer_session = add_capturing_session_to_server("Observer");
    // 2. Add the sending session.
    std::string sender_nickname = "SenderNick";
    auto sending_session = add_capturing_session_to_server(sender_nickname);
    std::string sender_id = sending_session->get_id();

    // Clear initial messages.
    observer_session->captured_messages.clear(); // Clear all previous
    sending_session->captured_messages.clear(); // Sender also receives its own message.

    // 3. Construct the original message JSON (as sent by a client, but processed by Session::on_read)
    std::string original_text = "Hello everyone!";
    json::object client_msg_payload = {
        {"type", "server_broadcast_message"}, // This is what Session constructs for server_.broadcast
        {"payload", {
            {"user_id", sender_id}, // Session adds this
            // Nickname is NOT YET HERE, ChatServer::broadcast adds it
            {"text", original_text},
            {"timestamp", Utils::getCurrentTimestampISO8601()} // Session adds this
        }}
    };
    std::string client_msg_json_str = json::serialize(client_msg_payload);

    // 4. Server broadcasts it.
    server_->broadcast(client_msg_json_str, sending_session);

    // 5. Check messages received by observer and sender.
    ASSERT_EQ(observer_session->captured_messages.size(), 1);
    std::string msg_to_observer = observer_session->captured_messages[0];
    EXPECT_TRUE(assert_json_message_basics(msg_to_observer, "server_broadcast_message", sender_id, sender_nickname));
    json::value jv_obs = json::parse(msg_to_observer);
    EXPECT_EQ(jv_obs.as_object().at("payload").as_object().at("text").as_string(), original_text);

    ASSERT_EQ(sending_session->captured_messages.size(), 1);
    std::string msg_to_sender = sending_session->captured_messages[0];
    EXPECT_TRUE(assert_json_message_basics(msg_to_sender, "server_broadcast_message", sender_id, sender_nickname));
    json::value jv_sender = json::parse(msg_to_sender);
    EXPECT_EQ(jv_sender.as_object().at("payload").as_object().at("text").as_string(), original_text);
}

TEST_F(ChatServerTest, SessionInternalNicknameLogic) {
    tcp::socket test_socket(ioc_);
    // Pass a dummy fixed ID to avoid issues if Session ID generation is non-deterministic or slow
    auto session_to_test_nick = std::make_shared<CapturingSession>(ioc_, std::move(test_socket), *server_, "test_session_id_001");

    std::string initial_nickname = session_to_test_nick->get_nickname();
    std::string default_nickname_prefix = "User"; // Default prefix from Session constructor
    ASSERT_TRUE(initial_nickname.rfind(default_nickname_prefix, 0) == 0 || initial_nickname.find(session_to_test_nick->get_id()) != std::string::npos)
        << "Default nickname should start with 'User' or contain session ID. Got: " << initial_nickname;

    std::string new_nick = "SuperCoder";
    session_to_test_nick->set_nickname(new_nick);
    EXPECT_EQ(session_to_test_nick->get_nickname(), new_nick);
}
```
tests/test_server_functionality.cpp created successfully.
```

Now, I'll modify `CMakeLists.txt` to include this new test file. I need to see its content first.
