#include <iostream>
#include <string_view>

namespace fw {

inline void printError(std::string_view msg) {
    std::cerr << "ERROR: " << msg << "\n";
}

} // namespace fw
