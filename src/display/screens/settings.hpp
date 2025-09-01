//
// Created by Assistant on 2025/1/27.
//

#ifndef CPP_MUD_OUC_SETTINGS_HPP
#define CPP_MUD_OUC_SETTINGS_HPP
#include "../display.hpp"

class SettingsScreen : public BaseScreen {
public:
    SettingsScreen();

    ftxui::Component GetComponent() override;
    
private:
    void HandleSelection(int selected_option);
    
    ftxui::Component component_;
    ftxui::Element content_element_; // 缓存内容元素
};


#endif //CPP_MUD_OUC_SETTINGS_HPP
