// =============================================
// 文件: display.hpp
// 描述: UI 层屏幕管理与导航接口声明，桥接核心 Game 与各屏幕组件。
// 说明: 仅声明接口；实现见 display.cpp 和 screens/*。
// =============================================
#ifndef DISPLAY_H
#define DISPLAY_H
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <map>
#include <string>
#include <vector>
#include <functional>
#include "../core/game.h"

// 导航请求的类型枚举：用于驱动屏幕切换或游戏动作
enum class NavigationAction {
    SWITCH_SCREEN,
    QUIT_GAME,
    START_NEW_GAME,
    LOAD_GAME,
    SAVE_GAME
};

// 导航请求结构：封装动作与可选的目标屏幕名
struct NavigationRequest {
    NavigationAction action;
    std::string target_screen;
    
    NavigationRequest(NavigationAction act, const std::string& target = "")
        : action(act), target_screen(target) {}
};

// 导航回调类型：各屏幕通过此回调向管理器发送导航请求
using NavigationCallback = std::function<void(const NavigationRequest&)>;

class BaseScreen {
public:
    virtual ~BaseScreen() = default;
    
    // 设置导航回调：由 ScreenManager 注入
    virtual void SetNavigationCallback(NavigationCallback callback) {
        navigation_callback_ = callback;
    }

protected:
    NavigationCallback navigation_callback_;

private:
    friend class ScreenManager;
    virtual ftxui::Component GetComponent() = 0;
};


class ScreenManager {
public:
    ScreenManager();
    ~ScreenManager();
    void mainloop();

private:
    // 处理来自屏幕的导航请求
    void HandleNavigationRequest(const NavigationRequest& request);
    ftxui::Component CreateMainContainer();
    void CreateNewScreen(); // 创建新的屏幕实例
    void SwitchToScreen(const std::string& screenName); // 切换到指定屏幕
    
    // 游戏状态管理方法（委托给 Game 类）
    void StartNewGame(); // 开始新游戏
    void LoadGame();     // 加载游戏
    void SaveGame();     // 保存游戏

    ftxui::ScreenInteractive* screen_; // 当前屏幕实例指针（由本类负责生命周期）
    std::map<std::string, BaseScreen*> screens_; // 名称到屏幕对象的映射
    std::string currentScreen_; // 当前屏幕名称
    std::string nextScreen_;    // 下一个要切换到的屏幕
    bool shouldQuit_ = false;   // 是否退出主循环
    bool shouldSwitchScreen_ = false; // 是否需要切换屏幕

    Game game_; // 游戏对象（核心逻辑）
};

#endif //DISPLAY_H