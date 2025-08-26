#pragma once
#include "../player/player.h"
#include <iostream>

class Display {
public:
    static void clearScreen();
    static void render(const Player& player);
    static void showMessage(const std::string& message);
};