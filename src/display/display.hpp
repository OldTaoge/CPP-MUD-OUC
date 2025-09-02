#ifndef DISPLAY_H
#define DISPLAY_H
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <map>
#include <string>
#include <vector>
#include <functional>

// 导航请求的类型枚举
enum class NavigationAction {
    SWITCH_SCREEN,
    QUIT_GAME
};

// 导航请求结构
struct NavigationRequest {
    NavigationAction action;
    std::string target_screen;
    
    NavigationRequest(NavigationAction act, const std::string& target = "")
        : action(act), target_screen(target) {}
};

// 导航回调类型
using NavigationCallback = std::function<void(const NavigationRequest&)>;

class BaseScreen {
public:
    virtual ~BaseScreen() = default;
    
    // 设置导航回调
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
    void HandleNavigationRequest(const NavigationRequest& request);
    ftxui::Component CreateMainContainer();
    void CreateNewScreen(); // 创建新的屏幕实例
    void SwitchToScreen(const std::string& screenName); // 切换到指定屏幕

    ftxui::ScreenInteractive* screen_; // 使用原始指针
    std::map<std::string, BaseScreen*> screens_;
    std::string currentScreen_;
    std::string nextScreen_; // 下一个要切换到的屏幕
    bool shouldQuit_ = false;
    bool shouldSwitchScreen_ = false; // 是否需要切换屏幕
    
    // 玩家对象指针
    class Player* player_;
    
    // 地图管理器指针
    class MapManager* mapManager_;
};

#endif //DISPLAY_H