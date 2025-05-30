#include "Game.h"
#include <iostream>

int main() {
    try {
        Game game;
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
         std::cerr << "An unknown error occurred." << std::endl;
         return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}