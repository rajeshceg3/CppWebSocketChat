// Utils.hpp
#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <time.h> // For strftime with gmtime

namespace Utils {

// Generates an ISO 8601 timestamp string in UTC
inline std::string getCurrentTimestampISO8601() {
    auto now = std::chrono::system_clock::now();
    auto itt = std::chrono::system_clock::to_time_t(now);

    std::ostringstream ss;
    // Use gmtime_r for thread-safety if available and on POSIX, otherwise gmtime
    // For C++20, std::format would be an option.
    // std::put_time requires #include <iomanip>
    // gmtime is not thread-safe on all platforms, use gmtime_s (Windows) or gmtime_r (POSIX) if possible.
    // For simplicity in this context, using std::gmtime. Production code might need more robust handling.
    std::tm buf;
    #ifdef _WIN32
        gmtime_s(&buf, &itt);
    #else
        gmtime_r(&itt, &buf); // POSIX
    #endif

    ss << std::put_time(&buf, "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

} // namespace Utils

#endif // UTILS_HPP
