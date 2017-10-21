#include "TestApp.h"

#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
    std::cout << "\n"
              << "Executing: " << argv[0] << "\n\n";
    TestApp app;
    try {
        app.run();
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}