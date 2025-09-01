//
// Created by Assistant on 2025/1/27.
//

#include "window_size_checker.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/screen_interactive.hpp>

namespace ftxui {

WindowSizeChecker::WindowSizeChecker(Component child, int min_width, int min_height)
    : child_(std::move(child)), min_width_(min_width), min_height_(min_height) {
    Add(child_);
}

Element WindowSizeChecker::OnRender() {
    auto screen = ScreenInteractive::Active();
    if (!screen) {
        return child_->Render();
    }
    
    // 获取当前终端大小
    auto terminal_size = Terminal::Size();
    int current_width = terminal_size.dimx;
    int current_height = terminal_size.dimy;
    
    // 检查窗口大小是否足够
    if (current_width < min_width_ || current_height < min_height_) {
        show_warning_ = true;
        return RenderWarning();
    } else {
        show_warning_ = false;
        return RenderContent();
    }
}

bool WindowSizeChecker::OnEvent(Event event) {
    if (show_warning_) {
        // 在警告模式下，只处理基本的退出事件
        if (event == Event::Escape || event == Event::CtrlC) {
            return true;
        }
        return false;
    }
    
    return child_->OnEvent(event);
}

Element WindowSizeChecker::RenderWarning() {
    std::string warning_text = "窗口大小不足！";
    std::string instruction = "请增大窗口大小以继续";
    std::string current_size = "当前: " + std::to_string(Terminal::Size().dimx) + "x" + std::to_string(Terminal::Size().dimy);
    std::string required_size = "需要: " + std::to_string(min_width_) + "x" + std::to_string(min_height_);
    std::string exit_hint = "按 ESC 或 Ctrl+C 退出";
    
    return vbox({
        text("") | flex,
        text(warning_text) | bold | color(Color::Red) | hcenter,
        text("") | flex,
        text(instruction) | color(Color::Yellow) | hcenter,
        text("") | flex,
        text(current_size) | color(Color::Blue) | hcenter,
        text(required_size) | color(Color::Blue) | hcenter,
        text("") | flex,
        text(exit_hint) | color(Color::GrayDark) | hcenter,
        text("") | flex
    }) | border | hcenter | vcenter;
}

Element WindowSizeChecker::RenderContent() {
    return child_->Render();
}

Component WindowSizeChecker::Make(Component child, int min_width, int min_height) {
    return ftxui::Make<WindowSizeChecker>(std::move(child), min_width, min_height);
}

} // namespace ftxui
