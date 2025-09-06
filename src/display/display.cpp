#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <iostream>
#include <map>
#include <string>
#include <thread>  // 用于sleep_for
#include <chrono>  // 用于时间相关操作

#include "screens/mainmenu.hpp"
#include "screens/illustrateMenu.hpp"
#include "screens/settings.hpp"
#include "screens/inventory.hpp"
#include "screens/gameplay.hpp"
#include "screens/map.hpp"
#include "screens/team.hpp"

using namespace ftxui;

ScreenManager::ScreenManager()
    : screen_(nullptr), // 初始化为nullptr
      currentScreen_("MainMenu"),
      nextScreen_(""),
      shouldQuit_(false),
      shouldSwitchScreen_(false),
      game_() // 初始化游戏对象
{
    // 创建导航回调
    auto nav_callback = [this](const NavigationRequest &request) {
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

    // 创建背包屏幕实例
    screens_["Inventory"] = new InventoryScreen(&game_);
    screens_["Inventory"]->SetNavigationCallback(nav_callback);

    // 创建游戏界面屏幕实例
    screens_["Gameplay"] = new GameplayScreen(&game_);
    screens_["Gameplay"]->SetNavigationCallback(nav_callback);

    // 创建地图界面屏幕实例
    screens_["Map"] = new MapScreen(&game_);
    screens_["Map"]->SetNavigationCallback(nav_callback);

    // 创建队伍配置屏幕实例
    screens_["Team"] = new TeamScreen(&game_);
    screens_["Team"]->SetNavigationCallback(nav_callback);

    // 创建第一个屏幕实例
    CreateNewScreen();
}

ScreenManager::~ScreenManager() {
    // 删除屏幕实例
    if (screen_) {
        delete screen_;
        screen_ = nullptr;
    }
    
    // 删除所有屏幕组件
    for (auto& pair : screens_) {
        delete pair.second;
    }
    screens_.clear();
}

void ScreenManager::HandleNavigationRequest(const NavigationRequest& request) {
    switch (request.action) {
        case NavigationAction::SWITCH_SCREEN:
            nextScreen_ = request.target_screen;
            shouldSwitchScreen_ = true;
            // 退出当前Loop，让mainloop能够处理屏幕切换
            if (screen_) {
                screen_->Exit();
            }
            break;
        case NavigationAction::START_NEW_GAME:
            StartNewGame();
            break;
        case NavigationAction::LOAD_GAME:
            LoadGame();
            break;
        case NavigationAction::SAVE_GAME:
            SaveGame();
            break;
        case NavigationAction::QUIT_GAME:
            shouldQuit_ = true;
            if (screen_) {
                screen_->Exit();
            }
            break;
    }
}

void ScreenManager::SwitchToScreen(const std::string& screenName) {
    // 检查目标屏幕是否存在
    if (screens_.count(screenName) == 0) {
        std::cerr << "Error: Target screen '" << screenName << "' not found! Available screens: ";
        for (const auto& pair : screens_) {
            std::cerr << "'" << pair.first << "' ";
        }
        std::cerr << std::endl;
        return; // 不进行切换，保持当前屏幕
    }
    
    // 删除旧的屏幕实例
    if (screen_) {
        // 先退出当前屏幕，确保所有任务都被清理
        screen_->Exit();
        
        // 等待更长时间确保所有延迟任务都被处理
        // 特别是动画任务的延迟任务需要足够时间来处理
        //std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        // 再次确保退出状态
        screen_->Exit();
        
        delete screen_;
    }
    
    // 更新当前屏幕名称
    currentScreen_ = screenName;
    
    // 如果切换到地图界面，更新地图数据
    if (screenName == "Map" && screens_.count("Map")) {
        MapScreen* mapScreen = dynamic_cast<MapScreen*>(screens_["Map"]);
        if (mapScreen) {
            mapScreen->UpdateMapData(game_);
        }
    }
    
    // 如果切换到游戏界面，更新游戏界面的地图显示
    if (screenName == "Gameplay" && screens_.count("Gameplay")) {
        GameplayScreen* gameplayScreen = dynamic_cast<GameplayScreen*>(screens_["Gameplay"]);
        if (gameplayScreen) {
            gameplayScreen->UpdateMapDisplay();
        }
    }
    
    // 创建新的屏幕实例
    screen_ = new ScreenInteractive(ScreenInteractive::Fullscreen());
    screen_->TrackMouse(true);
}

void ScreenManager::CreateNewScreen() {
    // 删除旧的屏幕实例
    if (screen_) {
        delete screen_;
    }
    
    // 创建新的屏幕实例
    screen_ = new ScreenInteractive(ScreenInteractive::Fullscreen());
    screen_->TrackMouse(true);
}

void ScreenManager::mainloop() {
    while (!shouldQuit_) {
        if (screens_.count(currentScreen_) && screen_) {
            screen_->Loop(screens_[currentScreen_]->GetComponent());
            
            // 检查是否需要切换屏幕
            if (shouldSwitchScreen_) {
                shouldSwitchScreen_ = false;
                SwitchToScreen(nextScreen_);
            }
        } else {
            std::cerr << "Error: Screen '" << currentScreen_ << "' not found!" << std::endl;
            break;
        }
    }
}

void ScreenManager::StartNewGame() {
    game_.StartNewGame();
    // 切换到游戏界面
    nextScreen_ = "Gameplay";
    shouldSwitchScreen_ = true;
    if (screen_) {
        screen_->Exit();
    }
}

void ScreenManager::LoadGame() {
    game_.LoadGame();
    // 切换到游戏界面
    nextScreen_ = "Gameplay";
    shouldSwitchScreen_ = true;
    if (screen_) {
        screen_->Exit();
    }
}

void ScreenManager::SaveGame() {
    game_.SaveGame();
}