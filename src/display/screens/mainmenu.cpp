//
// Created by Wentao on 2025/8/31.
//
#include <ftxui/component/component.hpp>
#include "../utils/utils.hpp"
#include "../window_size_checker.hpp"
#include <iostream>
#include "mainmenu.hpp"
#include "../display.hpp"
using namespace ftxui;
// 将您的字符画定义为一个原始字符串字面量
const std::string ascii_art_title = R"(
                                                                   .~.
                                                                   lo:
                                                                   )$[
                                                                   Q$J
                                                                   l8$W:
         u>                                   .cI                 X$$$u
       `n$k!                                 ^z$w;              .|@$$$B1
   "?jJa$$$8Ozjtttttttt/|\\|/tttttttttttttfvQ#$$$8Zn{l       ^-xk$$$$$$$br_^
   ^~\k$$$$$Bc1???]][[[fb$$m\[[[????][}}[[{\vp$$$hc\_"       `_|cm%$$$8Zv(+.
      |$$$$$k _j\[<:",,f&$$*{,,,?rf?:   """"":v$Z]ndO?  }Yn(_;""":}*$a?:"";(CCn1i`
      \$$$$$k.Q$$$$%0XXcYQQXzXYq$$$$Bpl_JCCJYhW$&$$$$$m>C$$$$%pYXzc#$*czYYk$$$$$BU`
      \$$$$$k.C$$$$$[^^^````^` t$$$$$z.'Zooo?m$$$BZQLQQ1J$$$$$x  lY@$@cI  X$$$$$Z^
      \$$$$$k.C$$$$$ZUUUUUUUUY!/$$$$$r .k$$$)Z$$$#:/jjt'J$$$$$d0o@$$$$$@aQk$$$$$U
      \$$$$$p C$$$$$]''......`'j$$$$$r .k$$$1Z$$$#<B$$$lU$$$$$QfJa$$$$$hJtm$$$$$J
      /$$$$$r Q$$$@oQYJpbbbbwYYb$$$$$/ .k$$$)Z$$$#<B$$@lU$$$$$v.',jB$%/,'.J$$$$$U
      Q$$$$q` \Lz/?l.',#$$$$w'._Lzt[!  .k$$${Z$$$#<B$$Q'J$$$$$pYYYca$kcYYYb$$$$$U
     1@$$$C:l}jUmhW%xl,o$$$$Z^!Y8#kZYf?~W$$8lm$$$#>dC[. Q$$WO1'  .[M$*-.  ')ZW$$J
     t@$$d1rh@$$$BMadmz_a$$$$O-UwdaWB$$$@$$#} w$$$M".    _)+" "?/Xd@$$$@pz\-^ ,+)+
  .~ZBkc+. 1mW@O{!".   `o$$$$0    '">(p@MZ\,  w$$$W^          l(Q%$$$$$$$&C)I
.>/}I.      "<]}}?+:  ^*$$$d>   I+]}}]i^     w$$%/             :J$$$$$X"
                       ^koC}'                0aci               'd$$$m.
                       ,'                     "'                 {$$$?
                                                                 "#$a'
                                                                  Q$Y
                                                                  t$)
                                                                  ?@<
                                                                  l#;
                                                                  "Y`
                                                                   .
)";





ScreenMainMenu::ScreenMainMenu() {
    // 1. 为选项添加数字前缀
    std::vector<std::string> base_entries = {
        "开始新游戏",
        "加载游戏",
        "游戏说明",
        "设置",
        "退出",
    };

    // 清空成员变量，然后填入带数字的新条目
    entries_.clear();
    for (size_t i = 0; i < base_entries.size(); ++i) {
        entries_.push_back(std::to_string(i + 1) + ". " + base_entries[i]);
    }

    // 创建按钮组件
    std::vector<Component> button_components;
    for (size_t i = 0; i < entries_.size(); ++i) {
        auto button = Button(entries_[i], [this, i] {
            this->HandleSelection(i);
        });
        button_components.push_back(button);
    }
    
    auto menu_container = Container::Vertical(button_components);
    
    // 预创建标题元素，避免每次渲染时重新创建
    auto title_lines = Utils::split_string(ascii_art_title);
    Elements title_elements;
    for (const auto& line : title_lines) {
        title_elements.push_back(text(line));
    }
    title_element_ = vbox(std::move(title_elements)) | hcenter;

    // 2. 重新设计UI布局
    auto main_renderer = Renderer(menu_container, [this, menu_container] {
        // 2.2 使用filler和border实现全屏边框和垂直居中效果
        return vbox({
                    title_element_,          // 使用预创建的标题元素
                    separator(),             // 分隔符
                    hbox(menu_container->Render() | hcenter) | hcenter |border,
               }) | border;               // 为整个屏幕添加边框
    });
    
    // 使用窗口大小检测包装主组件
    component_ = WindowSizeChecker::Make(main_renderer, 120, 50);
}

Component ScreenMainMenu::GetComponent() {
    return component_;
}

void ScreenMainMenu::HandleSelection(int selected_option) {
    if (!navigation_callback_) return;

    switch (selected_option) {
        case 0:
            // 开始新游戏
            navigation_callback_(NavigationRequest(NavigationAction::START_NEW_GAME));
            break;
        case 1:
            // 加载游戏
            navigation_callback_(NavigationRequest(NavigationAction::LOAD_GAME));
            break;
        case 2:
            navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Illustrate"));
            break;
        case 3:
            navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Settings"));
            break;
        case 4:
            navigation_callback_(NavigationRequest(NavigationAction::QUIT_GAME));
            break;
    }
}