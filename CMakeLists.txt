cmake_minimum_required (VERSION 3.0.0)
project (main CXX)

SET(CMAKE_EXPORT_COMPILE_COMMANDS 1)
# SET(CMAKE_CXX_COMPILER "clang++-18") # Use system default or user-specified compiler
SET(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES  ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

include_directories(include src) # Add src to include_directories for ChatClient.hpp
file(GLOB MAIN_SRC src/main.cpp) 
# Explicitly list client sources instead of using GLOB for more control
set(CLIENT_SRC
    src/ChatClient.cpp
    src/BoostWebSocketStream.cpp
)

# Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG    release-1.12.1 
)
FetchContent_MakeAvailable(googletest)
enable_testing()

add_executable(main ${MAIN_SRC} ${CLIENT_SRC}) # main uses ChatClient.cpp
target_link_libraries(main PRIVATE pthread)

# Test executable
add_executable(chat_client_tests tests/test_chat_client.cpp ${CLIENT_SRC})
target_link_libraries(chat_client_tests PRIVATE gtest_main gmock)

include(GoogleTest)
gtest_discover_tests(chat_client_tests)

add_custom_target(
	copy-compile-commands ALL
	${CMAKE_COMMAND} -E copy_if_different
	${CMAKE_BINARY_DIR}/compile_commands.json
	${CMAKE_CURRENT_LIST_DIR}
)
