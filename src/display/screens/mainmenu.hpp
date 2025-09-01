//
// Created by Wentao on 2025/8/31.
//

#ifndef CPP_MUD_OUC_MAINMENU_HPP
#define CPP_MUD_OUC_MAINMENU_HPP
#include "../display.hpp"

class ScreenMainMenu : public BaseScreen {
public:
    // 构造函数不再需要回调参数
    ScreenMainMenu();

    // 实现基类的虚函数
    ftxui::Component GetComponent() override;

private:
    void HandleSelection(int selected_option);
    
    ftxui::Component component_;
    std::vector<std::string> entries_;
    ftxui::Element title_element_; // 缓存标题元素
};


#endif //CPP_MUD_OUC_MAINMENU_HPP