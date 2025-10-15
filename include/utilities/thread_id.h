#pragma once

#include <iomanip>
#include <sstream>
#include <thread>

#define PORT 8080
#define BUFFER_SIZE 1024

// Function to convert a thread ID to a hexadecimal string
inline  std::string thread_id_to_hex(std::thread::id thread_id = std::this_thread::get_id(), const size_t num_bits = 16) {
    // Convert the thread ID to an unsigned long long for manipulation
    auto thread_id_value = std::hash<std::thread::id>{}(thread_id);

    // Mask the thread ID to get the lower num_bits bits
    uint64_t mask = (1ULL << num_bits) - 1;
    uint64_t short_id = thread_id_value & mask;
    
    // Convert the thread ID to a platform-dependent representation
    std::stringstream ss;
    ss << std::hex << short_id; // Convert to hex with zero-padding if necessary
    
    return ss.str();
}
