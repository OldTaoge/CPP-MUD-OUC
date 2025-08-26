#include "game.h"
#include "../display/display.h"

Game::Game()
    : player("Hero", 5, 5, 100), isRunning(false) {
    // 初始化菜单选项
    main_options = {
        "Move North (W)",
        "Move West (A)",
        "Move South (S)",
        "Move East (D)",
        "---", // 分隔符
        "Save Game",
        "Load Game",
        "---",
        "Exit",
    };
}

void Game::initialize() {
    isRunning = true;
    // 其他初始化...
}

void Game::quit() {
    isRunning = false;
}

void Game::run() {
    initialize();
    Display::gameLoop(*this); // 将游戏主循环交给Display模块
}