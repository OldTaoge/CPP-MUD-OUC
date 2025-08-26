#include "interaction.h"
#include "../storage/storage.h"
#include "../display/display.h" // 可能需要用于显示瞬时消息

void Interaction::processSelection(int option_index, Game& game, ftxui::ScreenInteractive& screen) {
    std::string selected = game.main_options[option_index];

    if (selected.find("North") != std::string::npos) {
        game.player.y--;
    } else if (selected.find("West") != std::string::npos) {
        game.player.x--;
    } else if (selected.find("South") != std::string::npos) {
        game.player.y++;
    } else if (selected.find("East") != std::string::npos) {
        game.player.x++;
    } else if (selected == "Save Game") {
        Storage::saveGame(game, "savegame.dat");
        // 可以在这里添加一个弹出消息
    } else if (selected == "Load Game") {
        Storage::loadGame(game, "savegame.dat");
    } else if (selected == "Exit") {
        game.quit();
        screen.Exit(); // 命令FTXUI循环退出
    }
    // 对于分隔符 "---"，不执行任何操作
}