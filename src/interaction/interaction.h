#pragma once
#include "../core/game.h"
#include <ftxui/component/screen_interactive.hpp>

class Interaction {
public:
    // 根据菜单选项索引处理逻辑
    static void processSelection(int option_index, Game &game, ftxui::ScreenInteractive &screen);
};