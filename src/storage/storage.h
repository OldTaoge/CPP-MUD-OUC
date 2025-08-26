#pragma once
#include "../core/game.h"

class Storage {
public:
    static void saveGame(const Game& game, const std::string& filename);
    static void loadGame(Game& game, const std::string& filename);
};