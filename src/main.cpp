#include "Game.h"
#include <iostream>

int main() {
    try {
        Game game;
        game.run();
    }
    catch (const std::exception& e) {
        // Lab: exception handling — catches any unhandled exceptions
        // from SFML, Box2D, or game code and reports them cleanly.
        std::cerr << "[Deadline's Edge] Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
