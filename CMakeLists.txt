cmake_minimum_required (VERSION 3.0.0)
project (main CXX)

SET(CMAKE_EXPORT_COMPILE_COMMANDS 1)
# SET(CMAKE_CXX_COMPILER "clang++-18") # Use system default or user-specified compiler
SET(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES  ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

include_directories(include src) # Add src to include_directories for server headers
file(GLOB MAIN_SRC src/main.cpp) 

# Explicitly list server sources
set(SERVER_SRC
    src/ChatServer.cpp
    src/Session.cpp
    # src/BoostWebSocketStream.cpp # If this is still needed for the server/session, keep it. Assuming not for now.
)

# Boost components
find_package(Boost 1.71.0 REQUIRED COMPONENTS system thread json) # Added json

add_executable(websocket-chat-server ${MAIN_SRC} ${SERVER_SRC}) # Renamed executable
# Link Boost libraries. For header-only parts of Boost like Asio and Beast,
# linking is mainly for components like system (for error_code), thread, and json.
if(Boost_FOUND)
  target_link_libraries(websocket-chat-server PRIVATE pthread Boost::system Boost::thread Boost::json) # Added Boost::json
else()
  target_link_libraries(websocket-chat-server PRIVATE pthread) # Fallback if Boost not found, though it's required
  message(WARNING "Boost libraries (system, thread, json) not found by find_package. Linking might be incomplete.")
endif()

# Google Test (Kept for now, but might need adjustment if tests targeted the client)
# If tests are specific to the client, they might be removed or refactored later.
# For now, let's assume we might want to add server tests in the future.
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG    release-1.12.1 
)
FetchContent_MakeAvailable(googletest)
enable_testing()

# Server tests
add_executable(server_tests tests/test_server_functionality.cpp ${SERVER_SRC})
target_include_directories(server_tests PRIVATE src) # Ensure tests can find server headers
target_link_libraries(server_tests PRIVATE gtest_main gmock Boost::system Boost::thread Boost::json)
include(GoogleTest)
gtest_discover_tests(server_tests)

# Remove old client test executable if it's no longer relevant or causes issues
if(TARGET chat_client_tests)
    remove_executable(chat_client_tests) # This is not a CMake function, illustrates intent.
                                        # Better to just comment out the add_executable for client tests
endif()

# Commenting out client tests for now as they target old client code
# add_executable(chat_client_tests tests/test_chat_client.cpp ${CLIENT_SRC}) # CLIENT_SRC is no longer defined
# target_link_libraries(chat_client_tests PRIVATE gtest_main gmock)
# include(GoogleTest)
# gtest_discover_tests(chat_client_tests)

add_custom_target(
	copy-compile-commands ALL
	${CMAKE_COMMAND} -E copy_if_different
	${CMAKE_BINARY_DIR}/compile_commands.json
	${CMAKE_CURRENT_LIST_DIR}
)
