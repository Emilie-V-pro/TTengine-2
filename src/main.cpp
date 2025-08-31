#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "app.hpp"
#include "engine.hpp"

int main() {
    fflush(stdout);
    TTe::App *app = new TTe::App();
    TTe::Engine engine{app};
    try {
        engine.init();
        engine.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}