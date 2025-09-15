// =============================================
// 文件: mainmenu.hpp
// 描述: 主菜单屏幕声明。提供开始/加载/说明/设置/退出等入口。
// =============================================

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
    // 处理按钮选择
    void HandleSelection(int selected_option);

    ftxui::Component component_;
    std::vector<std::string> entries_;
    ftxui::Element title_element_; // 缓存标题元素
};


#endif //CPP_MUD_OUC_MAINMENU_HPP