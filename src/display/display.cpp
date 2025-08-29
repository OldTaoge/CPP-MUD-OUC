#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include "display.hpp"
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace ftxui;

ScreenMainMenu::ScreenMainMenu(std::function<void(int)> on_selection) {
    entries_ = {
        "开始新游戏",
        "加载游戏",
        "游戏说明",
        "设置",
        "退出",
    };

    MenuOption option;
    // 当按下回车时，不再退出循环，而是调用传递进来的回调函数
    option.on_enter = [this, on_selection] {
        on_selection(selected_);
    };

    auto menu = Menu(&entries_, &selected_, option);

    // 将最终的渲染器赋值给成员变量 component_
    component_ = Renderer(menu, [menu] {
        return vbox({
                   text("我的文字冒险游戏") | bold | hcenter,
                   text(""),
                   menu->Render() | hcenter,
               }) |
               border |
               center;
    });
}

Component ScreenMainMenu::GetComponent() {
    return component_;
}


ScreenManager::ScreenManager()
    : screen_(ScreenInteractive::Fullscreen()), // 使用全屏模式
      currentScreen_("MainMenu")
{
    // 创建MainMenu屏幕实例
    // 将一个lambda函数作为回调传递给它
    // 这个lambda函数会调用ScreenManager自己的处理方法
    screens_["MainMenu"] = new ScreenMainMenu(
        [this](int selected) { this->HandleMainMenuSelection(selected); }
    );

    // 在这里可以初始化其他屏幕...
    // screens_["Settings"] = new ScreenSettings(...);
}

ScreenManager::~ScreenManager() {
    for (auto& pair : screens_) {
        delete pair.second;
    }
    screens_.clear();
}

// 这是从MainMenu回调过来的处理函数
void ScreenManager::HandleMainMenuSelection(int selected_option) {
    // 可以在这里根据用户的选择执行相应操作
    std::cout << "玩家选择了选项: " << selected_option << std::endl;
    switch (selected_option) {
        case 0:
            std::cout << "正在启动新游戏..." << std::endl;
            // currentScreen_ = "GamePlay"; // 切换到游戏界面
            break;
        case 1:
            std::cout << "正在加载游戏..." << std::endl;
            // currentScreen_ = "LoadGame"; // 切换到加载界面
            break;
        case 2:
            std::cout << "显示游戏说明..." << std::endl;
            // currentScreen_ = "Instructions";
            break;
        case 3:
            std::cout << "打开设置菜单..." << std::endl;
            // currentScreen_ = "Settings";
            break;
        case 4:
            std::cout << "退出游戏。" << std::endl;
            screen_.Exit(); // 正确的退出方式
            break;
    }
    // 如果切换了屏幕，Loop会自动刷新显示新的Component
    // 注意：在这个简单的例子中，除了退出，我们没有真正切换屏幕，
    // 所以选择后菜单会保持显示直到你按退出。
    // 在一个完整的游戏中，你会修改 currentScreen_ 的值来显示新界面。
}


void ScreenManager::mainloop() {
    if (screens_.count(currentScreen_)) {
        // 启动主循环，并渲染当前屏幕的组件
        screen_.Loop(screens_[currentScreen_]->GetComponent());
    } else {
        std::cerr << "Error: Screen '" << currentScreen_ << "' not found!" << std::endl;
    }
}