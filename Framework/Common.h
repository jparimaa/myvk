#include <iostream>
#include <string_view>

namespace fw {

inline void printError(std::string_view msg, const VkResult* result = nullptr) {
    std::cerr << "ERROR: " << msg << "\n";
    if (result != nullptr) {
        std::cerr << "Result: " << *result << "\n";
    }
}

} // namespace fw
