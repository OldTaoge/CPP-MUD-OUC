#ifndef DISPLAY_H
#define DISPLAY_H
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <map>
#include <string>
#include <vector>

using namespace ftxui;

class BaseScreen {
public:
    virtual ~BaseScreen() = default;

private:
    friend class ScreenManager;
    virtual ftxui::Component GetComponent() = 0;
};

class ScreenMainMenu : public BaseScreen {
public:
    // 构造函数接收一个回调，当用户做出选择时会调用这个回调
    explicit ScreenMainMenu(std::function<void(int)> on_selection);

    // 实现基类的虚函数
    ftxui::Component GetComponent() override;

private:
    ftxui::Component component_;
    std::vector<std::string> entries_;
    int selected_ = 0;
};

class ScreenManager {
public:
    ScreenManager();
    ~ScreenManager();
    void mainloop();

private:
    void HandleMainMenuSelection(int selected_option);

    ftxui::ScreenInteractive screen_;
    std::map<std::string, BaseScreen*> screens_;
    std::string currentScreen_;
};

#endif //DISPLAY_H