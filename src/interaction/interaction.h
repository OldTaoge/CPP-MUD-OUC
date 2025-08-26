#pragma once
#include <string>
#include "../core/game.h"

class Interaction {
public:
    static std::string getCommand();
    static void processCommand(const std::string& command, Game& game);
};