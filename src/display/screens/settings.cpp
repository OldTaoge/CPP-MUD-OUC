// =============================================
// 文件: settings.cpp
// 描述: 设置界面实现。提供AI开关等设置选项。
// =============================================

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>
#include <vector>
#include "../utils/utils.hpp"
#include "../utils/global_settings.hpp"
#include "settings.hpp"

using namespace ftxui;

SettingsScreen::SettingsScreen() : source_screen_("MainMenu") {
    // 初始化AI开关状态为全局设置的值
    ai_toggle_selected_ = GlobalSettings::IsAIEnabled() ? 1 : 0;
    
    // 创建AI开关组件
    ai_toggle_options_ = {"AI Disabled", "AI Enabled"};
    ai_toggle_ = Toggle(ai_toggle_options_, &ai_toggle_selected_);

    auto back_button = Button("返回", [this] {
        this->HandleSelection(0); // 0 表示返回
    });

    // 创建设置选项容器
    auto settings_container = Container::Vertical({
        ai_toggle_,
        back_button
    });

    component_ = Renderer(settings_container, [this, back_button] {
        return vbox({
                    text("设置") | hcenter | bold,
                    separator(),
                    text("AI智能建议:") | color(Color::Cyan),
                    ai_toggle_->Render(),
                    separator(),
                    text("注意: AI建议的响应速度会受到互联网连接影响；AI建议的回答可能不准确，请谨慎使用。；") | 
                        color(Color::GrayLight) | dim,
                    separator(),
                    back_button->Render() | hcenter,
                }) | border;
    });
}

ftxui::Component SettingsScreen::GetComponent() {
    return component_;
}

void SettingsScreen::SetSourceScreen(const std::string& source_screen) {
    source_screen_ = source_screen;
}

void SettingsScreen::HandleSelection(int selected_option) {
    if (!navigation_callback_) return;
    
    switch (selected_option) {
        case 0: // 返回到来源界面
            // 在返回前更新全局设置
            UpdateGlobalSettings();
            navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, source_screen_));
            break;
        default:
            break;
    }
}

void SettingsScreen::UpdateGlobalSettings() {
    // 将当前设置状态同步到全局设置
    GlobalSettings::SetAIEnabled(ai_toggle_selected_ == 1);
}
