#include "storage.h"
#include "../display/display.h"
#include <fstream>
#include <iostream>

void Storage::saveGame(const Game& game, const std::string& filename) {
    std::ofstream saveFile(filename);
    if (saveFile.is_open()) {
        saveFile << game.player.name << std::endl;
        saveFile << game.player.x << std::endl;
        saveFile << game.player.y << std::endl;
        saveFile << game.player.health << std::endl;
        saveFile.close();
        Display::showMessage("Game saved successfully.");
    } else {
        Display::showMessage("Error: Unable to save game.");
    }
}

void Storage::loadGame(Game& game, const std::string& filename) {
    std::ifstream loadFile(filename);
    if (loadFile.is_open()) {
        loadFile >> game.player.name;
        loadFile >> game.player.x;
        loadFile >> game.player.y;
        loadFile >> game.player.health;
        loadFile.close();
        Display::showMessage("Game loaded successfully.");
    } else {
        Display::showMessage("Error: Unable to load game. Starting a new game.");
        game.initialize();
    }
}