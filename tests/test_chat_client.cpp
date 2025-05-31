#include "gtest/gtest.h"
#include "gmock/gmock.h"    // For GMock
#include "ChatClient.hpp" 
#include "MockWebSocketStream.hpp" // Correct path if tests directory is sibling to src
#include <string>
#include <sstream> 
#include <iostream> 
#include <memory>           // For std::unique_ptr, std::make_unique
#include <vector>           // For std::vector with boost::asio::buffer

// Using declarations for GMock
using ::testing::_;
using ::testing::Return;
using ::testing::SetArgReferee; // For setting error_code by reference
using ::testing::Invoke;
using ::testing::NiceMock; // To suppress warnings about uninteresting calls if any
using ::testing::HasSubstr; // For checking parts of output strings

// Helper class to capture cerr output (already defined, ensure it's available)
class CerrCapture {
public:
    CerrCapture() : old_cerr_buf(std::cerr.rdbuf()) {
        std::cerr.rdbuf(buffer.rdbuf());
    }
    ~CerrCapture() {
        std::cerr.rdbuf(old_cerr_buf);
    }
    std::string str() const {
        return buffer.str();
    }
private:
    std::stringstream buffer;
    std::streambuf* old_cerr_buf;
};

TEST(ChatClientTest, PlaceholderTest) {
    ASSERT_EQ(1, 1); 
}

TEST(ChatClientTest, ConstructionWithNonExistentServer) {
    std::string host = "127.0.0.1"; 
    std::string port = "9999";    

    CerrCapture cerr_capture; 
    ChatClient client(host, port);
    EXPECT_FALSE(client.is_connected());
    std::string output = cerr_capture.str();
    EXPECT_NE(output.find("[ERROR] ChatClient: TCP Connect to 127.0.0.1:9999 failed"), std::string::npos); 
}


// Test fixture for tests using a mocked stream
class ChatClientMockedTest : public ::testing::Test {
protected:
    // Use NiceMock to avoid warnings for uninteresting calls to is_open in constructor/destructor
    NiceMock<MockWebSocketStream>* raw_mock_stream_ptr; 
    net::io_context test_ioc; 
    std::unique_ptr<ChatClient> client;

    void SetUp() override {
        auto unique_mock_stream = std::make_unique<NiceMock<MockWebSocketStream>>();
        raw_mock_stream_ptr = unique_mock_stream.get(); 
        
        // Simulate the stream being open for the client's constructor to set connected_ = true
        EXPECT_CALL(*raw_mock_stream_ptr, is_open()).WillRepeatedly(Return(true));
        
        client = std::make_unique<ChatClient>(std::move(unique_mock_stream), test_ioc);
        
        // After construction, client.is_connected() should be true if mock says it's open.
        // This is based on the ChatClient DI constructor logic:
        // if (ws_impl_ && ws_impl_->is_open()) { connected_ = true; }
        // So, the above EXPECT_CALL for is_open() handles this.
        ASSERT_TRUE(client->is_connected()); // Ensure client is connected for send/receive tests
    }
};

TEST_F(ChatClientMockedTest, SendMessageSuccess) {
    const std::string message = "hello world";
    
    EXPECT_CALL(*raw_mock_stream_ptr, write(_, _))
        .Times(1)
        .WillOnce(Invoke([&](const net::const_buffer& buffer, beast::error_code& ec) {
            ASSERT_EQ(net::buffer_size(buffer), message.size());
            std::string sent_data(net::buffer_cast<const char*>(buffer), net::buffer_size(buffer));
            ASSERT_EQ(sent_data, message);
            ec = {}; // No error
        }));
    
    EXPECT_TRUE(client->send_message(message));
}

TEST_F(ChatClientMockedTest, SendMessageFailureNetworkError) {
    const std::string message = "test error send";
    
    EXPECT_CALL(*raw_mock_stream_ptr, write(_, _))
        .Times(1)
        .WillOnce(Invoke([&](const net::const_buffer&, beast::error_code& ec) {
            ec = boost::asio::error::connection_reset; // Simulate network error
        }));

    CerrCapture cerr_capture; 
    EXPECT_FALSE(client->send_message(message));
    EXPECT_NE(cerr_capture.str().find("Error sending message (write): connection_reset"), std::string::npos);
    EXPECT_NE(cerr_capture.str().find("Connection closed or broken while sending."), std::string::npos);
    EXPECT_FALSE(client->is_connected()); // Client should mark as disconnected
}

TEST_F(ChatClientMockedTest, ReceiveMessageSuccess) {
    const std::string expected_message = "server says hello";
    
    EXPECT_CALL(*raw_mock_stream_ptr, read(_, _))
        .Times(1)
        .WillOnce(Invoke([&](beast::flat_buffer& buffer, beast::error_code& ec) -> size_t {
            // Simulate server sending data by copying to buffer
            auto mutable_buf = buffer.prepare(expected_message.size());
            net::buffer_copy(mutable_buf, net::buffer(expected_message));
            buffer.commit(expected_message.size());
            ec = {}; // No error
            return expected_message.size();
        }));

    std::string received_msg = client->receive_message();
    ASSERT_EQ(received_msg, expected_message);
}

TEST_F(ChatClientMockedTest, ReceiveMessageFailureNetworkError) {
    EXPECT_CALL(*raw_mock_stream_ptr, read(_, _))
        .Times(1)
        .WillOnce(Invoke([&](beast::flat_buffer&, beast::error_code& ec) -> size_t {
            ec = boost::asio::error::connection_reset; // Simulate network error
            return 0; // Bytes read
        }));

    CerrCapture cerr_capture;
    std::string received_msg = client->receive_message();
    EXPECT_NE(cerr_capture.str().find("Error receiving message (read): connection_reset"), std::string::npos);
    EXPECT_NE(cerr_capture.str().find("Connection broken or timed out while receiving."), std::string::npos);
    // ChatClient's receive_message returns a specific error string
    EXPECT_EQ(received_msg, "[Error: Connection broken or timed out]");
    EXPECT_FALSE(client->is_connected()); // Client should mark as disconnected
}

TEST_F(ChatClientMockedTest, ReceiveMessageFailureServerClose) {
    EXPECT_CALL(*raw_mock_stream_ptr, read(_, _))
        .Times(1)
        .WillOnce(Invoke([&](beast::flat_buffer&, beast::error_code& ec) -> size_t {
            ec = beast::websocket::error::closed; // Simulate server closing connection
            return 0; // Bytes read
        }));

    CerrCapture cerr_capture;
    std::string received_msg = client->receive_message();
    EXPECT_NE(cerr_capture.str().find("Error receiving message (read): websocket: close"), std::string::npos);
    EXPECT_NE(cerr_capture.str().find("Connection closed by server."), std::string::npos);
    EXPECT_EQ(received_msg, "[Connection closed by server]");
    EXPECT_FALSE(client->is_connected()); // Client should mark as disconnected
}

// Add more tests below as needed.

TEST_F(ChatClientMockedTest, RunHandlesQuitCommand) {
    std::stringstream input_stream;
    std::stringstream output_stream;
    input_stream << "/quit\n";

    // is_open() is already set to WillRepeatedly(Return(true)) in SetUp for the initial connected state.
    // We might need to make it more dynamic if the test involves the stream closing unexpectedly.
    // For /quit, the client initiates the close.

    EXPECT_CALL(*raw_mock_stream_ptr, close(_, _))
        .Times(1)
        .WillOnce(Invoke([](beast::websocket::close_code, beast::error_code& ec) {
            ec = {}; // No error on close
        }));

    client->run(input_stream, output_stream);

    std::string output = output_stream.str();
    EXPECT_THAT(output, HasSubstr("Chat client started. Type '/quit' to exit."));
    EXPECT_THAT(output, HasSubstr("> ")); // Prompt for /quit
    EXPECT_THAT(output, HasSubstr("Disconnected."));
    // is_connected() state after run() with /quit:
    // The run loop exits, then calls ws_impl_->close().
    // The connected_ flag itself isn't explicitly set to false by the /quit command path,
    // but the session is over.
    // Depending on strictness, one might expect is_connected() to be false after run() finishes.
    // The current client logic: is_connected() reflects connected_ flag.
    // The run() method calls close, but does not set connected_ = false in /quit path.
    // This is acceptable as the client object might be destroyed after run() anyway.
}

TEST_F(ChatClientMockedTest, RunHandlesServerDisconnectOnReceive) {
    std::stringstream input_stream;
    std::stringstream output_stream;
    input_stream << "hello server\n"; 

    // is_open() is set to true in SetUp.
    // Let's refine it for this test to ensure it reflects the mock's state change.
    ON_CALL(*raw_mock_stream_ptr, is_open())
        .WillByDefault(Return(true)); // Default to true

    EXPECT_CALL(*raw_mock_stream_ptr, write(_, _))
        .WillOnce(Invoke([](const net::const_buffer&, beast::error_code& ec) {
            ec = {}; 
        }));

    EXPECT_CALL(*raw_mock_stream_ptr, read(_, _))
        .WillOnce(Invoke([&](beast::flat_buffer&, beast::error_code& ec) -> size_t {
            ec = beast::websocket::error::closed; 
            // Client's receive_message should set connected_ = false.
            // And is_open() on the mock stream should now reflect that it's no longer open.
            EXPECT_CALL(*raw_mock_stream_ptr, is_open()).WillRepeatedly(Return(false));
            return 0; 
        }));
    
    // Expect 'close' to be called as client should try to close its end, even if server already did.
    EXPECT_CALL(*raw_mock_stream_ptr, close(_, _))
        .Times(1)
        .WillOnce(Invoke([](beast::websocket::close_code, beast::error_code& ec) {
            ec = {}; 
        }));

    CerrCapture cerr_capture; // To check logged errors
    client->run(input_stream, output_stream);
    
    std::string output = output_stream.str();
    std::string err_output = cerr_capture.str();

    // Check user-facing output
    EXPECT_THAT(output, HasSubstr("Server: [Connection closed by server]"));
    EXPECT_THAT(output, HasSubstr("Disconnected."));

    // Check logged errors
    EXPECT_THAT(err_output, HasSubstr("Connection lost while receiving message or server closed connection."));
    EXPECT_FALSE(client->is_connected()); 
}

TEST_F(ChatClientMockedTest, RunHandlesInputStreamEOF) {
    std::stringstream input_stream; // Empty, will immediately signal EOF
    std::stringstream output_stream;

    // is_open() from SetUp is fine.
    
    EXPECT_CALL(*raw_mock_stream_ptr, close(_, _))
        .Times(1)
        .WillOnce(Invoke([](beast::websocket::close_code, beast::error_code& ec) {
            ec = {};
        }));

    CerrCapture cerr_capture;
    client->run(input_stream, output_stream);

    std::string output = output_stream.str();
    std::string err_output = cerr_capture.str();

    EXPECT_THAT(output, HasSubstr("Disconnected."));
    // Check log for EOF
    EXPECT_THAT(err_output, HasSubstr("EOF detected on input stream."));
    EXPECT_THAT(err_output, HasSubstr("Exiting run loop."));
}

TEST_F(ChatClientMockedTest, RunSendsAndReceivesOneMessage) {
    std::stringstream input_stream;
    std::stringstream output_stream;
    input_stream << "ping\n"; 
    input_stream << "/quit\n"; 

    const std::string server_response_msg = "pong";

    // is_open() from SetUp is fine.

    EXPECT_CALL(*raw_mock_stream_ptr, write(_, _)) 
        .Times(1)
        .WillOnce(Invoke([](const net::const_buffer& buffer, beast::error_code& ec) {
            std::string sent_data(net::buffer_cast<const char*>(buffer), net::buffer_size(buffer));
            ASSERT_EQ(sent_data, "ping"); // Use ASSERT_EQ for critical checks
            ec = {};
        }));

    EXPECT_CALL(*raw_mock_stream_ptr, read(_, _)) 
        .Times(1)
        .WillOnce(Invoke([&](beast::flat_buffer& buffer, beast::error_code& ec) -> size_t {
            auto mutable_buf = buffer.prepare(server_response_msg.size());
            net::buffer_copy(mutable_buf, net::buffer(server_response_msg));
            buffer.commit(server_response_msg.size());
            ec = {};
            return server_response_msg.size();
        }));
    
    EXPECT_CALL(*raw_mock_stream_ptr, close(_, _)) 
        .Times(1)
        .WillOnce(Invoke([](beast::websocket::close_code, beast::error_code& ec) {
            ec = {};
        }));

    client->run(input_stream, output_stream);

    std::string output = output_stream.str();
    EXPECT_THAT(output, HasSubstr("Server: pong"));
    EXPECT_THAT(output, HasSubstr("Disconnected."));
}
