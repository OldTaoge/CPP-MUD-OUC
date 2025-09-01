//
// Created by Wentao on 2025/8/31.
//
#include <ftxui/component/component.hpp>
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

// 辅助函数，用于按行分割字符串
std::vector<std::string> split_string(const std::string& str) {
    std::vector<std::string> lines;
    std::stringstream ss(str);
    std::string line;
    while (std::getline(ss, line)) {
        lines.push_back(line);
    }
    return lines;
}



ScreenMainMenu::ScreenMainMenu(std::function<void(int)> on_selection) {
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

    MenuOption option;
    option.on_enter = [this, on_selection] {
        on_selection(selected_);
    };
    auto menu = Menu(&entries_, &selected_, option);

    // 2. 重新设计UI布局
    component_ = Renderer(menu, [menu] {
        // --- 开始修改 ---
        // 将字符画按行分割
        auto title_lines = split_string(ascii_art_title);
        // 为每一行创建一个 text 元素，并把它们放入一个垂直容器 (vbox)
        Elements title_elements;
        for (const auto& line : title_lines) {
            title_elements.push_back(text(line));
        }
        auto title_vbox = vbox(std::move(title_elements));
        // --- 结束修改 ---

        // 将标题和菜单组合成一个垂直框
        auto content_box = vbox({
            title_vbox | hcenter, // 使用字符画标题
            text(""),             // 分隔符
            menu->Render(),
        });

        // 2.2 使用filler和border实现全屏边框和垂直居中效果
        return vbox({
                   filler(),                  // 上方的弹性空间
                   content_box | hcenter,     // 中间的内容 (水平居中)
                   filler(),                  // 下方的弹性空间
               }) |
               border; // 为整个屏幕添加边框
    });
}

Component ScreenMainMenu::GetComponent() {
    return component_;
}