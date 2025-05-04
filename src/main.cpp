#include "Game.h"
#include <iostream>

int main() {
    try {
        Game game;
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        // Optional: Pause execution so the user can see the error in the console
        // std::cin.get();
        return EXIT_FAILURE;
    } catch (...) {
         std::cerr << "An unknown error occurred." << std::endl;
         // std::cin.get();
         return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}