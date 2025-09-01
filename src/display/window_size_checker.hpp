//
// Created by Assistant on 2025/1/27.
//

#ifndef CPP_MUD_OUC_WINDOW_SIZE_CHECKER_HPP
#define CPP_MUD_OUC_WINDOW_SIZE_CHECKER_HPP

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/screen_interactive.hpp>

namespace ftxui {

class WindowSizeChecker : public ComponentBase {
public:
    WindowSizeChecker(Component child, int min_width, int min_height);
    
    Element OnRender() override;
    bool OnEvent(Event event) override;
    
    static Component Make(Component child, int min_width, int min_height);

private:
    Component child_;
    int min_width_;
    int min_height_;
    bool show_warning_ = false;
    
    Element RenderWarning();
    Element RenderContent();
};

} // namespace ftxui

#endif //CPP_MUD_OUC_WINDOW_SIZE_CHECKER_HPP
