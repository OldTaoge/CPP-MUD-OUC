// =============================================
// 文件: settings.hpp
// 描述: 设置界面声明。当前提供返回功能与占位内容。
// =============================================

#ifndef CPP_MUD_OUC_SETTINGS_HPP
#define CPP_MUD_OUC_SETTINGS_HPP
#include "../display.hpp"

class SettingsScreen : public BaseScreen {
public:
    SettingsScreen();
    
    // 设置来源界面，用于确定返回的目标
    void SetSourceScreen(const std::string& source_screen);

    ftxui::Component GetComponent() override;
    
private:
    void HandleSelection(int selected_option);
    
    ftxui::Component component_;
    ftxui::Element content_element_; // 缓存内容元素
    std::string source_screen_; // 来源界面名称
};


#endif //CPP_MUD_OUC_SETTINGS_HPP
