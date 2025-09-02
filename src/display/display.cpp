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
#include "screens/gameplay.hpp"
#include "screens/inventory.hpp"
#include "screens/questscreen.hpp"
#include "screens/mapScreen.hpp"
#include "../player/player.h"
#include "../core/map.h"

using namespace ftxui;

ScreenManager::ScreenManager()
    : screen_(nullptr), // 初始化为nullptr
      currentScreen_("MainMenu"),
      nextScreen_(""),
      shouldQuit_(false),
      shouldSwitchScreen_(false),
      player_(new Player("旅行者", 0, 0, 100)),
      mapManager_(new MapManager())
{
    // 创建导航回调
    auto nav_callback = [this](const NavigationRequest& request) {
        this->HandleNavigationRequest(request);
    };
    
    // 创建MainMenu屏幕实例
    screens_["MainMenu"] = new ScreenMainMenu();
    screens_["MainMenu"]->SetNavigationCallback(nav_callback);
    
    // 创建游戏主界面实例
    screens_["Gameplay"] = new GameplayScreen(player_);
    screens_["Gameplay"]->SetNavigationCallback(nav_callback);
    // 设置地图管理器
    static_cast<GameplayScreen*>(screens_["Gameplay"])->SetMapManager(mapManager_);
    
    // 创建游戏说明屏幕实例
    screens_["Illustrate"] = new IllustrateMenu();
    screens_["Illustrate"]->SetNavigationCallback(nav_callback);
    
    // 创建设置屏幕实例
    screens_["Settings"] = new SettingsScreen();
    screens_["Settings"]->SetNavigationCallback(nav_callback);
    
    // 创建背包屏幕实例
    screens_["Inventory"] = new InventoryScreen(player_);
    screens_["Inventory"]->SetNavigationCallback(nav_callback);
    
    // 创建任务屏幕实例
    screens_["Quest"] = new QuestScreen(player_);
    screens_["Quest"]->SetNavigationCallback(nav_callback);
    
    // 创建地图屏幕实例
    screens_["Map"] = new MapScreen(mapManager_);
    screens_["Map"]->SetNavigationCallback(nav_callback);
    
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
    
    // 删除玩家对象
    if (player_) {
        delete player_;
        player_ = nullptr;
    }
    
    // 删除地图管理器
    if (mapManager_) {
        delete mapManager_;
        mapManager_ = nullptr;
    }
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
        case NavigationAction::QUIT_GAME:
            shouldQuit_ = true;
            if (screen_) {
                screen_->Exit();
            }
            break;
    }
}

void ScreenManager::SwitchToScreen(const std::string& screenName) {
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