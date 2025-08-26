#include "interaction.h"
#include "../storage/storage.h"
#include "../display/display.h"

std::string Interaction::getCommand() {
    std::string command;
    std::cout << "> ";
    std::cin >> command;
    return command;
}

void Interaction::processCommand(const std::string& command, Game& game) {
    if (command == "w") {
        game.player.y--;
    } else if (command == "s") {
        game.player.y++;
    } else if (command == "a") {
        game.player.x--;
    } else if (command == "d") {
        game.player.x++;
    } else if (command == "save") {
        Storage::saveGame(game, "savegame.dat");
    } else if (command == "load") {
        Storage::loadGame(game, "savegame.dat");
    } else if (command == "exit") {
        game.isRunning = false;
    } else {
        Display::showMessage("Unknown command.");
    }
    Display::clearScreen();
}