// =============================================
// 文件: display.cpp
// 描述: 屏幕管理器实现。负责多屏注册、导航、以及与 Game 的桥接。
// 说明: 屏幕组件在 screens/* 下定义，此处只做装配与流程控制。
// =============================================
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
#include "screens/save_select.hpp"
#include "screens/shop.hpp"

using namespace ftxui;

ScreenManager::ScreenManager()
    : screen_(nullptr), // 初始化为 nullptr
      currentScreen_("MainMenu"),
      nextScreen_("")
{
    // 创建导航回调
    auto nav_callback = [this](const NavigationRequest &request) {
        this->HandleNavigationRequest(request);
    };

    // 创建 MainMenu 屏幕实例
    screens_["MainMenu"] = new ScreenMainMenu();
    screens_["MainMenu"]->SetNavigationCallback(nav_callback);

    // 创建 游戏说明 屏幕实例
    screens_["Illustrate"] = new IllustrateMenu();
    screens_["Illustrate"]->SetNavigationCallback(nav_callback);

    // 创建 设置 屏幕实例
    screens_["Settings"] = new SettingsScreen();
    screens_["Settings"]->SetNavigationCallback(nav_callback);

    // 创建 背包 屏幕实例
    screens_["Inventory"] = new InventoryScreen(&game_);
    screens_["Inventory"]->SetNavigationCallback(nav_callback);

    // 创建 游戏界面 屏幕实例
    screens_["Gameplay"] = new GameplayScreen(&game_);
    screens_["Gameplay"]->SetNavigationCallback(nav_callback);

    // 创建 地图界面 屏幕实例
    screens_["Map"] = new MapScreen(&game_);
    screens_["Map"]->SetNavigationCallback(nav_callback);

    // 创建 队伍配置 屏幕实例
    screens_["Team"] = new TeamScreen(&game_);
    screens_["Team"]->SetNavigationCallback(nav_callback);

    // 创建 商店 屏幕实例
    screens_["Shop"] = new ShopScreen(&game_);
    screens_["Shop"]->SetNavigationCallback(nav_callback);

    // 创建 存档选择 屏幕实例（加载模式）
    screens_["SaveLoad"] = new SaveSelectScreen(&game_, SaveSelectMode::LOAD);
    screens_["SaveLoad"]->SetNavigationCallback(nav_callback);

    // 创建 存档选择 屏幕实例（保存模式）
    screens_["SaveSave"] = new SaveSelectScreen(&game_, SaveSelectMode::SAVE);
    screens_["SaveSave"]->SetNavigationCallback(nav_callback);

    // 创建第一个屏幕实例
    CreateNewScreen();
}

ScreenManager::~ScreenManager() {
    // 删除屏幕实例与所有屏幕组件
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
            // 如果切换到设置页面，设置来源界面
            if (request.target_screen == "Settings") {
                SettingsScreen* settingsScreen = dynamic_cast<SettingsScreen*>(screens_["Settings"]);
                if (settingsScreen) {
                    settingsScreen->SetSourceScreen(currentScreen_);
                }
            }
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
            nextScreen_ = "SaveLoad";
            shouldSwitchScreen_ = true;
            if (screen_) {
                screen_->Exit();
            }
            break;
        case NavigationAction::SAVE_GAME: {
            // 设置保存存档页面的来源界面
            SaveSelectScreen* saveScreen = dynamic_cast<SaveSelectScreen*>(screens_["SaveSave"]);
            if (saveScreen) {
                saveScreen->SetSourceScreen(currentScreen_);
            }
            nextScreen_ = "SaveSave";
            shouldSwitchScreen_ = true;
            if (screen_) {
                screen_->Exit();
            }
            break;
        }
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
    
    // 如果切换到游戏界面，更新游戏界面的显示信息
    if (screenName == "Gameplay" && screens_.count("Gameplay")) {
        GameplayScreen* gameplayScreen = dynamic_cast<GameplayScreen*>(screens_["Gameplay"]);
        if (gameplayScreen) {
            gameplayScreen->UpdateMapDisplay();
            // 更新玩家信息显示
            gameplayScreen->UpdatePlayerInfo(game_.getPlayer());
            
            // 更新队伍状态显示
            gameplayScreen->RefreshTeamDisplay();
            
            // 根据游戏状态添加相应的欢迎消息
            if (game_.getCurrentState() == GameState::PLAYING) {
                // 检查是否是新游戏（通过检查玩家等级和经验判断）
                const auto& player = game_.getPlayer();
                if (player.level == 1 && player.experience == 0) {
                    gameplayScreen->UpdateGameStatus("欢迎来到新的冒险！");
                    gameplayScreen->UpdateGameStatus("使用 WASD 移动，空格键交互，T 打开工具菜单");
                } else {
                    gameplayScreen->UpdateGameStatus("游戏数据已加载完成");
                }
            }
        }
    }

    // 如果切换到队伍界面，刷新队伍数据
    if (screenName == "Team" && screens_.count("Team")) {
        TeamScreen* teamScreen = dynamic_cast<TeamScreen*>(screens_["Team"]);
        if (teamScreen) {
            teamScreen->Refresh();
        }
    }
    
    // 如果切换到存档选择界面，刷新存档列表
    if ((screenName == "SaveLoad" || screenName == "SaveSave") && screens_.count(screenName)) {
        SaveSelectScreen* saveScreen = dynamic_cast<SaveSelectScreen*>(screens_[screenName]);
        if (saveScreen) {
            saveScreen->RefreshSaveList();
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
    
    // 清空游戏界面的历史消息
    if (screens_.count("Gameplay")) {
        GameplayScreen* gameplayScreen = dynamic_cast<GameplayScreen*>(screens_["Gameplay"]);
        if (gameplayScreen) {
            gameplayScreen->ClearAllMessages();
        }
    }
    
    // 切换到游戏界面
    nextScreen_ = "Gameplay";
    shouldSwitchScreen_ = true;
    if (screen_) {
        screen_->Exit();
    }
}

void ScreenManager::LoadGame() {
    // 清空游戏界面的历史消息
    if (screens_.count("Gameplay")) {
        GameplayScreen* gameplayScreen = dynamic_cast<GameplayScreen*>(screens_["Gameplay"]);
        if (gameplayScreen) {
            gameplayScreen->ClearAllMessages();
        }
    }
    
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