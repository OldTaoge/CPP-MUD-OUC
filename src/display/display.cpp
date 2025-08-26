#include "Display.h"

void Display::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void Display::render(const Player& player) {
    // 在这里绘制游戏地图和玩家信息
    std::cout << "--------------------" << std::endl;
    std::cout << "Player: " << player.name << std::endl;
    std::cout << "Health: " << player.health << std::endl;
    std::cout << "Position: (" << player.x << ", " << player.y << ")" << std::endl;
    std::cout << "--------------------" << std::endl;
    showMessage("Enter your command (w/a/s/d to move, save to save, load to load, exit to quit):");
}

void Display::showMessage(const std::string& message) {
    std::cout << message << std::endl;
}