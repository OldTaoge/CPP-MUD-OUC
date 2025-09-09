#include "gameplay.hpp"
#include "../../core/game.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/screen/terminal.hpp>
#include <sstream>
#include <algorithm>

GameplayScreen::GameplayScreen(Game* game) : game_(game) {
    // 初始化工具选项
    tool_options_ = {
        "背包",
        "队伍",
        "地图",
        "设置",
        "保存游戏"
    };
    
    // 初始化地图显示
    current_map_lines_ = {"Loading map..."};
    current_block_info_ = "Initializing...";
    
    // 如果游戏对象存在，立即更新地图
    if (game_) {
        UpdateMapDisplay();
    }
    
    // 初始化玩家基础数值，避免未定义显示
    player_level_ = 0;
    player_experience_ = 0;

    // 创建UI组件
    chat_input_ = ftxui::Input(&chat_input_buffer_, "输入聊天消息...");
    game_input_ = ftxui::Input(&game_input_buffer_, "输入游戏命令...");
    
    // 工具按钮
    tool_button_ = ftxui::Button("工具", [this] {
        ShowToolOverlay();
    });
    
    // 关闭按钮
    close_button_ = ftxui::Button("关闭", [this] {
        HideToolOverlay();
    });
    
    // 创建工具选项按钮
    for (size_t i = 0; i < tool_options_.size(); ++i) {
        int index = i; // 捕获索引
        tool_option_buttons_.push_back(
            ftxui::Button(tool_options_[i], [this, index] {
                HandleToolOption(index);
            })
        );
    }
    
    // 工具叠加图层已集成到主组件中，不再需要单独的组件

    // 底部可点击操作按钮（鼠标/键盘）
    bottom_action_buttons_.clear();
    bottom_action_buttons_.push_back(ftxui::Button("交互", [this] { HandleGameCommand("interact"); }));
    bottom_action_buttons_.push_back(ftxui::Button("背包", [this] {
        if (navigation_callback_) navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Inventory"));
    }));
    bottom_action_buttons_.push_back(ftxui::Button("队伍", [this] {
        if (navigation_callback_) navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Team"));
    }));
    bottom_action_buttons_.push_back(ftxui::Button("地图", [this] {
        if (navigation_callback_) navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Map"));
    }));
    bottom_action_buttons_.push_back(ftxui::Button("设置", [this] {
        if (navigation_callback_) navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Settings"));
    }));
    bottom_action_buttons_.push_back(ftxui::Button("保存", [this] {
        if (navigation_callback_) navigation_callback_(NavigationRequest(NavigationAction::SAVE_GAME));
    }));

    // 顶栏：左工具按钮 / 中间标题 / 右侧简要状态
    auto header_container = ftxui::Container::Horizontal({ tool_button_ });
    auto header_renderer = ftxui::Renderer(header_container, [this] {
        auto dims = ftxui::Terminal::Size();
        int header_h = std::max(3, dims.dimy / 10); // 顶栏约 10%
        std::string right_status = "Lv." + std::to_string(player_level_) +
                                   "  HP: " + std::to_string(player_hp_) + "/" + std::to_string(player_max_hp_);
        auto left = tool_button_->Render();
        auto center_title = ftxui::text("原神MUD版") | ftxui::bold;
        auto right = ftxui::text(right_status);
        auto bar = ftxui::hbox({
                   left,
                   ftxui::filler(),
                   center_title,
                   ftxui::filler(),
                   right,
               }) | ftxui::border;
        return bar | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, header_h);
    });

    // 中部：左“游戏消息”/右“游戏状态（详细）”
    auto main_renderer = ftxui::Renderer([this] {
        auto dims = ftxui::Terminal::Size();
        int header_h = std::max(3, dims.dimy / 10);
        int bottom_h = std::max(3, dims.dimy / 10);
        int main_h = std::max(5, dims.dimy - header_h - bottom_h);
        int message_h = std::max(5, main_h * 6 / 10); // 为消息预留约 60% 的高度

        // 左侧：地图显示
        std::vector<ftxui::Element> left;
        left.push_back(ftxui::text("当前区域") | ftxui::bold);
        left.push_back(ftxui::separator());
        for (const auto& line : current_map_lines_) left.push_back(ftxui::text(line));
        if (!current_block_info_.empty()) {
            left.push_back(ftxui::separator());
            left.push_back(ftxui::text(current_block_info_));
        }
        auto left_box = ftxui::vbox(left) | ftxui::border;

        // 右侧：游戏消息（固定高度视口） + 游戏状态（玩家/队伍）
        std::vector<ftxui::Element> right;
        right.push_back(ftxui::text("游戏消息") | ftxui::bold);
        right.push_back(ftxui::separator());
        std::vector<ftxui::Element> msg_lines;
        if (game_messages_.empty()) {
            msg_lines.push_back(ftxui::text("暂无消息") | ftxui::color(ftxui::Color::GrayLight));
        } else {
            for (const auto& m : game_messages_) msg_lines.push_back(ftxui::text(m));
        }
        auto messages_view = ftxui::vbox(msg_lines) | ftxui::vscroll_indicator | ftxui::yframe |
                             ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, message_h);
        right.push_back(messages_view);
        right.push_back(ftxui::separator());
        right.push_back(ftxui::text("游戏状态") | ftxui::bold);
        right.push_back(ftxui::separator());
        right.push_back(ftxui::text("玩家: " + player_name_));
        right.push_back(ftxui::text("Lv." + std::to_string(player_level_)));
        right.push_back(ftxui::text("HP: " + std::to_string(player_hp_) + "/" + std::to_string(player_max_hp_)));
        right.push_back(ftxui::text(player_status_));
        right.push_back(ftxui::separator());
        right.push_back(ftxui::text("队伍成员:") | ftxui::bold)
        ;
        if (team_members_.empty()) {
            right.push_back(ftxui::text("暂无队伍成员") | ftxui::color(ftxui::Color::GrayLight));
        } else {
            for (const auto& member : team_members_) right.push_back(ftxui::text("• " + member));
        }
        auto right_box = ftxui::vbox(right) | ftxui::border;

        // 按 6:4 比例分配宽度
        int left_w = std::max(20, dims.dimx * 6 / 10);
        int right_w = std::max(10, dims.dimx - left_w);
        auto left_sized = left_box | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, left_w);
        auto right_sized = right_box | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, right_w);

        return ftxui::hbox({ left_sized, right_sized }) | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, main_h);
    });

    // 底部：游戏操作选项（按钮 + 提示）
    auto bottom_container = ftxui::Container::Horizontal(bottom_action_buttons_);
    auto bottom_renderer = ftxui::Renderer(bottom_container, [this] {
        auto dims = ftxui::Terminal::Size();
        int bottom_h = std::max(3, dims.dimy / 10); // 底栏约 10%
        std::vector<ftxui::Element> btns;
        for (auto& b : bottom_action_buttons_) {
            btns.push_back(b->Render());
            btns.push_back(ftxui::text(" "));
        }
        auto help = ftxui::text("W/A/S/D 移动  空格 交互  T 工具菜单  Q/E 切换队友");
        auto bar = ftxui::hbox({ ftxui::hbox(btns), ftxui::filler(), help }) | ftxui::border;
        return bar | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, bottom_h);
    });

    // 主组件（顶栏 / 中部 / 底部）
    component_ = ftxui::Container::Vertical({ header_renderer, main_renderer, bottom_renderer });

    // 将工具菜单以叠加层形式集成
    std::vector<ftxui::Component> final_components;
    final_components.push_back(component_);
    auto tool_menu_component = ftxui::Renderer([this] {
        if (show_tool_overlay_) {
            std::vector<ftxui::Element> elements;
            elements.push_back(ftxui::text("=== 工具菜单 ===") | ftxui::bold | ftxui::center);
            elements.push_back(ftxui::separator());
            for (size_t i = 0; i < tool_options_.size(); ++i) {
                auto text = tool_options_[i];
                if (i == selected_tool_button_) {
                    text = "> " + text;
                    elements.push_back(ftxui::text(text) | ftxui::color(ftxui::Color::Yellow));
                } else {
                    text = "  " + text;
                    elements.push_back(ftxui::text(text));
                }
            }
            elements.push_back(ftxui::separator());
            elements.push_back(ftxui::text("按 Enter 选择，ESC 取消"));
            return ftxui::vbox(elements) | ftxui::border | ftxui::center;
        }
        return ftxui::text("");
    });
    final_components.push_back(tool_menu_component);
    component_ = ftxui::Container::Vertical(final_components);
    
    // 设置键盘事件处理
    component_ |= ftxui::CatchEvent([this](ftxui::Event event) {
        if (show_tool_overlay_) {
            if (event == ftxui::Event::ArrowUp) {
                selected_tool_button_ = std::max(0, selected_tool_button_ - 1);
                return true;
            } else if (event == ftxui::Event::ArrowDown) {
                selected_tool_button_ = std::min((int)tool_options_.size() - 1, selected_tool_button_ + 1);
                return true;
            } else if (event == ftxui::Event::Return) {
                HandleToolOption(selected_tool_button_);
                return true;
            } else if (event == ftxui::Event::Escape) {
                HideToolOverlay();
                return true;
            }
        } else {
            // 游戏控制
            if (event == ftxui::Event::Character('t') || event == ftxui::Event::Character('T')) {
                ShowToolOverlay();
                return true;
            } else if (event == ftxui::Event::Character('w') || event == ftxui::Event::ArrowUp) {
                HandleGameCommand("move north");
                return true;
            } else if (event == ftxui::Event::Character('s') || event == ftxui::Event::ArrowDown) {
                HandleGameCommand("move south");
                return true;
            } else if (event == ftxui::Event::Character('a') || event == ftxui::Event::ArrowLeft) {
                HandleGameCommand("move west");
                return true;
            } else if (event == ftxui::Event::Character('d') || event == ftxui::Event::ArrowRight) {
                HandleGameCommand("move east");
                return true;
            } else if (event == ftxui::Event::Character(' ')) {
                HandleGameCommand("interact");
                return true;
            } else if (event == ftxui::Event::Character('q') || event == ftxui::Event::Character('Q')) {
                HandleGameCommand("switch_next_member");
                return true;
            } else if (event == ftxui::Event::Character('e') || event == ftxui::Event::Character('E')) {
                HandleGameCommand("switch_prev_member");
                return true;
            }
        }
        return false;
    });
}

ftxui::Component GameplayScreen::GetComponent() {
    return component_;
}

void GameplayScreen::UpdatePlayerInfo(const Player& player) {
    player_name_ = player.name;
    auto active = player.getActiveMember();
    player_hp_ = active ? active->getCurrentHealth() : 0;
    player_max_hp_ = active ? active->getTotalHealth() : 0;
    player_level_ = player.level;
    player_experience_ = player.experience;
    
    std::stringstream ss;
    ss << "等级: " << player.level << " 经验: " << player.experience;
    player_status_ = ss.str();
}

void GameplayScreen::AddChatMessage(const std::string& message) {
    chat_messages_.push_back(message);
    // 限制消息数量
    if (chat_messages_.size() > 20) {
        chat_messages_.erase(chat_messages_.begin());
    }
}

void GameplayScreen::UpdateGameStatus(const std::string& status) {
    game_messages_.push_back(status);
    // 限制消息数量
    if (game_messages_.size() > 20) {
        game_messages_.erase(game_messages_.begin());
    }
}

void GameplayScreen::UpdateTeamStatus(const std::vector<std::string>& teamMembers) {
    team_members_ = teamMembers;
}

void GameplayScreen::RefreshTeamDisplay() {
    if (!game_) return;
    
    std::vector<std::string> teamMemberNames;
    const auto& player = game_->getPlayer();
    for (const auto& member : player.teamMembers) {
        if (member) {
            std::string memberInfo = member->getName() + " (HP: " + 
                                   std::to_string(member->getCurrentHealth()) + "/" + 
                                   std::to_string(member->getTotalHealth()) + ")";
            teamMemberNames.push_back(memberInfo);
        }
    }
    UpdateTeamStatus(teamMemberNames);
}

void GameplayScreen::ClearAllMessages() {
    game_messages_.clear();
    chat_messages_.clear();
}

void GameplayScreen::UpdateMapDisplay() {
    if (!game_) {
        current_map_lines_ = {"Map not available"};
        current_block_info_ = "Game not initialized";
        return;
    }
    
    try {
        // 获取当前区块地图
        const auto& mapManager = game_->getMapManager();
        current_map_lines_ = mapManager.renderCurrentBlock();
        
        // 获取当前位置信息
        current_block_info_ = mapManager.getCurrentCellInfo();
        
    } catch (const std::exception& e) {
        current_map_lines_ = {"Map error"};
        current_block_info_ = "Error: " + std::string(e.what());
    }
}

void GameplayScreen::HandleToolButton(int buttonIndex) {
    if (buttonIndex >= 0 && buttonIndex < tool_options_.size()) {
        HandleToolOption(buttonIndex);
    }
}

void GameplayScreen::HandleGameCommand(const std::string& command) {
    if (!game_) {
        UpdateGameStatus("游戏对象未初始化");
        return;
    }
    
    if (command == "move north") {
        if (game_->movePlayer(0, -1)) {
            // UpdateGameStatus("向北移动");
            UpdatePlayerInfo(game_->getPlayer());
            UpdateMapDisplay();  // 实时更新地图
        } else {
            UpdateGameStatus("无法向北移动");
        }
    } else if (command == "move south") {
        if (game_->movePlayer(0, 1)) {
            // UpdateGameStatus("向南移动");
            UpdatePlayerInfo(game_->getPlayer());
            UpdateMapDisplay();  // 实时更新地图
        } else {
            UpdateGameStatus("无法向南移动");
        }
    } else if (command == "move west") {
        if (game_->movePlayer(-1, 0)) {
            // UpdateGameStatus("向西移动");
            UpdatePlayerInfo(game_->getPlayer());
            UpdateMapDisplay();  // 实时更新地图
        } else {
            UpdateGameStatus("无法向西移动");
        }
    } else if (command == "move east") {
        if (game_->movePlayer(1, 0)) {
            // UpdateGameStatus("向东移动");
            UpdatePlayerInfo(game_->getPlayer());
            UpdateMapDisplay();  // 实时更新地图
        } else {
            UpdateGameStatus("无法向东移动");
        }
    } else if (command == "switch_next_member") {
        auto& player = game_->getPlayer();
        if (player.switchToNextActiveMember()) {
            UpdatePlayerInfo(player);
            auto activeMember = player.getActiveMember();
            if (activeMember) {
                UpdateGameStatus("切换到队友: " + activeMember->getName());
            }
            
            // 更新队伍状态显示
            RefreshTeamDisplay();
        } else {
            UpdateGameStatus("无法切换队友");
        }
    } else if (command == "switch_prev_member") {
        auto& player = game_->getPlayer();
        if (player.switchToPreviousActiveMember()) {
            UpdatePlayerInfo(player);
            auto activeMember = player.getActiveMember();
            if (activeMember) {
                UpdateGameStatus("切换到队友: " + activeMember->getName());
            }
            
            // 更新队伍状态显示
            RefreshTeamDisplay();
        } else {
            UpdateGameStatus("无法切换队友");
        }
    } else if (command == "interact") {
        auto interactions = game_->getAvailableMapInteractions();
        if (!interactions.empty()) {
            // 尝试第一个可用的交互
            auto result = game_->interactWithMap(interactions[0]);
            UpdateGameStatus(result.message);
            if (result.success) {
                UpdatePlayerInfo(game_->getPlayer());
                UpdateMapDisplay();  // 交互后更新地图
                
                // 更新队伍状态显示（重要：处理新角色加入的情况）
                RefreshTeamDisplay();
            }
        } else {
            UpdateGameStatus("当前位置没有可交互的内容");
        }
    } else {
        UpdateGameStatus("执行命令: " + command);
    }
}

void GameplayScreen::ShowToolOverlay() {
    show_tool_overlay_ = true;
    selected_tool_button_ = 0;
    UpdateGameStatus("工具菜单已打开");
}

void GameplayScreen::HideToolOverlay() {
    show_tool_overlay_ = false;
    UpdateGameStatus("工具菜单已关闭");
}

void GameplayScreen::HandleToolOption(int optionIndex) {
    if (optionIndex < 0 || optionIndex >= tool_options_.size()) {
        UpdateGameStatus("无效的工具选项索引: " + std::to_string(optionIndex));
        HideToolOverlay();
        return;
    }
    
    std::string option = tool_options_[optionIndex];
    UpdateGameStatus("选择了工具: " + option);
    
    try {
        if (option == "背包") {
            if (navigation_callback_) {
                navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Inventory"));
            } else {
                UpdateGameStatus("错误: 导航回调未设置");
            }
        } else if (option == "队伍") {
            if (navigation_callback_) {
                navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Team"));
            } else {
                UpdateGameStatus("错误: 导航回调未设置");
            }
        } else if (option == "地图") {
            if (navigation_callback_) {
                navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Map"));
            } else {
                UpdateGameStatus("错误: 导航回调未设置");
            }
        } else if (option == "设置") {
            if (navigation_callback_) {
                navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Settings"));
            } else {
                UpdateGameStatus("错误: 导航回调未设置");
            }
        } else if (option == "保存游戏") {
            if (navigation_callback_) {
                navigation_callback_(NavigationRequest(NavigationAction::SAVE_GAME));
            } else {
                UpdateGameStatus("错误: 导航回调未设置");
            }
        } else {
            UpdateGameStatus("未知的工具选项: " + option);
        }
    } catch (const std::exception& e) {
        UpdateGameStatus("处理工具选项时出错: " + std::string(e.what()));
    }
    
    HideToolOverlay();
}
