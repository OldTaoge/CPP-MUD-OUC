#pragma once
#include "../player/player.h"
#include <vector>
#include <string>

class Game {
public:
    Player player;
    bool isRunning;

    // 新增：游戏主菜单的选项
    std::vector<std::string> main_options;
    int selected_option = 0; // 当前选中的选项索引

    Game();
    void initialize();
    void run();
    void quit();
};