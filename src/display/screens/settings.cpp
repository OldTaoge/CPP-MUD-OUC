//
// Created by Assistant on 2025/1/27.
//

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>
#include "../utils/utils.hpp"
#include "settings.hpp"

using namespace ftxui;

SettingsScreen::SettingsScreen() {
    std::string content = "暂无设置可选";

    auto back_button = Button("返回主菜单", [this] {
        this->HandleSelection(0); // 0 表示返回主菜单
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

void SettingsScreen::HandleSelection(int selected_option) {
    if (!navigation_callback_) return;
    
    switch (selected_option) {
        case 0: // 返回主菜单
            navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "MainMenu"));
            break;
        default:
            break;
    }
}
