#include "display.h"
#include "../interaction/interaction.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

void Display::gameLoop(Game& game) {
    auto screen = ScreenInteractive::Fullscreen();

    // --- 交互逻辑处理 ---
    // 当在菜单上按下回车键时，执行此回调
    // 1. 先创建一个配置对象
    auto menu_options = MenuOption();

    // 2. 在配置对象上设置 on_enter
    menu_options.on_enter = [&] { /* ... */ };

    // 3. 创建 Menu 时，把配置对象传进去
    auto menu = Menu(&game.main_options, &game.selected_option, menu_options);

    // --- 界面布局 ---
    auto layout = Container::Horizontal({
        menu,
    });

    auto main_renderer = Renderer(layout, [&] {
        // --- 绘制玩家状态窗口 ---
        auto player_stats = window(text("Player Info"),
            vbox({
                text("Name: " + game.player.name),
                text("Health: " + std::to_string(game.player.health)),
                text("Position: (" + std::to_string(game.player.x) + ", " + std::to_string(game.player.y) + ")"),
            })
        ) | flex;

        // --- 绘制主菜单窗口 ---
        auto main_menu = window(text("Actions"),
            menu->Render()
        ) | size(WIDTH, LESS_THAN, 30); // 限制宽度

        return hbox({
            player_stats,
            separator(),
            main_menu,
        }) | border;
    });

    // 启动事件循环
    screen.Loop(main_renderer);
}