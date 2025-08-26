#include "Game.h"
#include "../display/display.h"
#include "../interaction/interaction.h"

Game::Game() : player("Hero", 5, 5, 100), isRunning(false) {}

void Game::initialize() {
    isRunning = true;
    // 初始化游戏世界，加载地图等
}

void Game::update() {
    // 游戏逻辑更新，例如NPC移动、事件触发等
}

void Game::run() {
    initialize();
    Display::clearScreen();

    while (isRunning) {
        Display::render(player);
        std::string command = Interaction::getCommand();
        Interaction::processCommand(command, *this);
        update();
    }
}