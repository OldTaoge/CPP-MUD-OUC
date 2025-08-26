#pragma once
#include "../core/game.h"
#include <ftxui/component/screen_interactive.hpp> // 引入FTXUI头文件

class Display {
public:
    static void gameLoop(Game& game);
};