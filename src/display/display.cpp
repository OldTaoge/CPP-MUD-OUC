#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <sstream> // 用于字符串分割

#include "screens/mainmenu.hpp"

using namespace ftxui;

ScreenManager::ScreenManager()
    : screen_(ScreenInteractive::Fullscreen()), // 使用全屏模式
      currentScreen_("MainMenu")
{
    // 创建MainMenu屏幕实例
    screens_["MainMenu"] = new ScreenMainMenu(
        [this](int selected) { this->HandleMainMenuSelection(selected); }
    );
}

ScreenManager::~ScreenManager() {
    for (auto& pair : screens_) {
        delete pair.second;
    }
    screens_.clear();
}

void ScreenManager::HandleMainMenuSelection(int selected_option) {
    switch (selected_option) {
        case 0:
            std::cout << "正在启动新游戏..." << std::endl;
            break;
        case 1:
            std::cout << "正在加载游戏..." << std::endl;
            break;
        case 2:
            std::cout << "显示游戏说明..." << std::endl;
            break;
        case 3:
            std::cout << "打开设置菜单..." << std::endl;
            break;
        case 4:
            screen_.Exit();
            break;
    }
}


void ScreenManager::mainloop() {
    if (screens_.count(currentScreen_)) {
        screen_.Loop(screens_[currentScreen_]->GetComponent());
    } else {
        std::cerr << "Error: Screen '" << currentScreen_ << "' not found!" << std::endl;
    }
}