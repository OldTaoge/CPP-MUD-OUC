// =============================================
// 文件: illustrateMenu.cpp
// 描述: 游戏说明界面实现。展示内容与返回主菜单按钮。
// =============================================

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>
#include "../utils/utils.hpp"
#include "illustrateMenu.hpp"

using namespace ftxui;

IllustrateMenu::IllustrateMenu() {
    std::string content =
        "《原神》——文字版 单机MUD\n\n"
        "- 背景：旅行者降临提瓦特大陆，结识伙伴，踏上寻找真相的旅途。\n"
        "- 玩法：通过键盘选择菜单，阅读剧情，做出抉择以影响走向。\n"
        "- 操作：\n"
        "  · 在菜单中使用 上/下 方向键或 W/S 移动光标。\n"
        "  · 回车 确认选择。\n"
        "  · Esc/Backspace 返回上一级（本界面提供『返回主菜单』按钮）。\n\n"
        "- 存档：在关键节点可选择保存/读取进度（功能将陆续开放）。\n"
        "- 提示：建议全屏游玩以获得更佳阅读体验。\n";

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
                    text("游戏说明") | hcenter,
                    separator(),
                    content_element_,
                    separator(),
                    back_button->Render() | hcenter,
                }) | border;
    });
}

ftxui::Component IllustrateMenu::GetComponent() {
    return component_;
}

void IllustrateMenu::HandleSelection(int selected_option) {
    if (!navigation_callback_) return;
    
    switch (selected_option) {
        case 0: // 返回主菜单
            navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "MainMenu"));
            break;
        default:
            break;
    }
}