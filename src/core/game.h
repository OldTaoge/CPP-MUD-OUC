#pragma once
#include "../player/player.h"
#include <vector>

class Game {
public:
    Player player;
    bool isRunning;
    // 可以添加地图、NPC列表等
    // std::vector<std::vector<char>> map;

    Game();
    void initialize();
    void update();
    void run();
};