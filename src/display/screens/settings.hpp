// =============================================
// 文件: settings.hpp
// 描述: 设置界面声明。提供AI开关等设置选项。
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
    void UpdateGlobalSettings();
    
    ftxui::Component component_;
    ftxui::Component ai_toggle_;
    std::string source_screen_; // 来源界面名称
    int ai_toggle_selected_ = 0; // AI开关状态，0=禁用，1=启用
    std::vector<std::string> ai_toggle_options_; // AI开关选项
};


#endif //CPP_MUD_OUC_SETTINGS_HPP
