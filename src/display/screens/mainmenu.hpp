//
// Created by Wentao on 2025/8/31.
//

#ifndef CPP_MUD_OUC_MAINMENU_HPP
#define CPP_MUD_OUC_MAINMENU_HPP
#include "../display.hpp"

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


#endif //CPP_MUD_OUC_MAINMENU_HPP