#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <sstream> // 用于字符串分割

#include "screens/mainmenu.hpp"
#include "screens/illustrateMenu.hpp"
#include "screens/settings.hpp"

using namespace ftxui;

ScreenManager::ScreenManager()
    : screen_(ScreenInteractive::FullscreenPrimaryScreen()), // 使用全屏模式
      currentScreen_("MainMenu"),
      shouldQuit_(false)
{
    // 创建导航回调
    auto nav_callback = [this](const NavigationRequest& request) {
        this->HandleNavigationRequest(request);
    };
    
    // 创建MainMenu屏幕实例
    screens_["MainMenu"] = new ScreenMainMenu();
    screens_["MainMenu"]->SetNavigationCallback(nav_callback);
    
    // 创建游戏说明屏幕实例
    screens_["Illustrate"] = new IllustrateMenu();
    screens_["Illustrate"]->SetNavigationCallback(nav_callback);
    
    // 创建设置屏幕实例
    screens_["Settings"] = new SettingsScreen();
    screens_["Settings"]->SetNavigationCallback(nav_callback);
}

ScreenManager::~ScreenManager() {
    for (auto& pair : screens_) {
        delete pair.second;
    }
    screens_.clear();
}

void ScreenManager::HandleNavigationRequest(const NavigationRequest& request) {
    switch (request.action) {
        case NavigationAction::SWITCH_SCREEN:
            currentScreen_ = request.target_screen;
            screen_.Exit();
            // screen_ = ScreenInteractive::Fullscreen();
            break;
        case NavigationAction::QUIT_GAME:
            shouldQuit_ = true;
            screen_.Exit();
            break;
    }
}

void ScreenManager::mainloop() {
    while (!shouldQuit_) {
        if (screens_.count(currentScreen_)) {
            screen_.Loop(screens_[currentScreen_]->GetComponent());
        } else {
            std::cerr << "Error: Screen '" << currentScreen_ << "' not found!" << std::endl;
            break;
        }
    }
}