// =============================================
// 文件: settings.cpp
// 描述: 设置界面实现。当前仅包含返回按钮与占位文本。
// =============================================

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>
#include "../utils/utils.hpp"
#include "settings.hpp"

using namespace ftxui;

SettingsScreen::SettingsScreen() : source_screen_("MainMenu") {
    std::string content = "暂无设置可选";

    auto back_button = Button("返回", [this] {
        this->HandleSelection(0); // 0 表示返回
    });

    // 预创建内容元素，避免每次渲染时重新创建
    auto content_lines = Utils::split_string(content);
    Elements content_elements;
    for (const auto& line : content_lines) {
        content_elements.push_back(text(line));
    }
    content_element_ = vbox(std::move(content_elements));

    component_ = Renderer(back_button, [this, back_button] {
        return vbox({
                    text("设置") | hcenter,
                    separator(),
                    content_element_ | hcenter,
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
            navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, source_screen_));
            break;
        default:
            break;
    }
}
