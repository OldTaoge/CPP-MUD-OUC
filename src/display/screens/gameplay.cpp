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
        
        // 右侧状态信息 - 使用更美观的显示
        std::string right_status = "Lv." + std::to_string(player_level_) +
                                   "  HP:" + std::to_string(player_hp_) + "/" + std::to_string(player_max_hp_);
        
        auto left = tool_button_->Render();
        auto center_title = ftxui::text("原神MUD版") | ftxui::bold | ftxui::color(ftxui::Color::Yellow);
        auto right = ftxui::text(right_status) | ftxui::color(ftxui::Color::Cyan);
        
        auto bar = ftxui::hbox({
                   left,
                   ftxui::filler(),
                   center_title,
                   ftxui::filler(),
                   right,
               }) | ftxui::border | ftxui::color(ftxui::Color::Blue);
        return bar | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, header_h);
    });

    // 中部：左“游戏消息”/右“游戏状态（详细）”
    auto main_renderer = ftxui::Renderer([this] {
        auto dims = ftxui::Terminal::Size();
        int header_h = std::max(3, dims.dimy / 10);
        int bottom_h = std::max(3, dims.dimy / 10);
        int main_h = std::max(5, dims.dimy - header_h - bottom_h);
        int message_h = std::max(5, main_h * 6 / 10); // 为消息预留约 60% 的高度

        // 左侧：地图显示 - 优化美观度
        std::vector<ftxui::Element> left;
        
        // 地图标题 - 显示当前区块名称
        std::string currentBlockName = "未知区域";
        auto currentBlock = game_->getMapManager().getCurrentBlock();
        if (currentBlock) {
            currentBlockName = currentBlock->getName();
        }
        left.push_back(ftxui::text("[地图] " + currentBlockName) | ftxui::bold | ftxui::color(ftxui::Color::Cyan));
        left.push_back(ftxui::separator());
        
        // 地图内容 - 使用等宽字体和颜色，为交互元素添加精确的单位置高亮
        std::vector<ftxui::Element> map_content;
        for (const auto& line : current_map_lines_) {
            // 检查是否为地图网格区域（包含边框字符的行，但不是图例或标题）
            bool isMapGrid = (line.find("+") != std::string::npos || line.find("-") != std::string::npos || 
                             line.find("|") != std::string::npos) && 
                             line.find("=== 地图图例 ===") == std::string::npos && 
                             line.find("P =") == std::string::npos && line.find("I =") == std::string::npos && 
                             line.find("N =") == std::string::npos && line.find("S =") == std::string::npos && 
                             line.find("M =") == std::string::npos && line.find("^v>< =") == std::string::npos && 
                             line.find("# =") == std::string::npos && line.find(". =") == std::string::npos &&
                             line.length() > 5; // 确保不是简短的标题行，但允许区块名称显示
            
            if (isMapGrid) {
                // 地图网格区域 - 为每个字符位置单独应用样式
                std::vector<ftxui::Element> styled_chars;
                for (size_t i = 0; i < line.length(); ++i) {
                    char c = line[i];
                    ftxui::Element char_element = ftxui::text(std::string(1, c));
                    
                    // 根据字符类型应用不同的样式，确保良好的对比度和可读性
                    if (c == '+' || c == '-' || c == '|') {
                        // 边框字符 - 青色
                        char_element = char_element | ftxui::color(ftxui::Color::Cyan) | ftxui::bold;
                    } else if (c == 'P') {
                        // 玩家位置 - 白色文字配深蓝色背景
                        char_element = char_element | ftxui::color(ftxui::Color::White) | ftxui::bgcolor(ftxui::Color::Blue) | ftxui::bold;
                    } else if (c == 'I') {
                        // 物品 - 白色文字配深绿色背景
                        char_element = char_element | ftxui::color(ftxui::Color::White) | ftxui::bgcolor(ftxui::Color::Green) | ftxui::bold;
                    } else if (c == 'N') {
                        // NPC - 白色文字配深青色背景
                        char_element = char_element | ftxui::color(ftxui::Color::White) | ftxui::bgcolor(ftxui::Color::Cyan) | ftxui::bold;
                    } else if (c == 'S') {
                        // 神像 - 黑色文字配黄色背景
                        char_element = char_element | ftxui::color(ftxui::Color::Black) | ftxui::bgcolor(ftxui::Color::Yellow) | ftxui::bold;
                    } else if (c == 'M') {
                        // 怪物 - 白色文字配红色背景
                        char_element = char_element | ftxui::color(ftxui::Color::White) | ftxui::bgcolor(ftxui::Color::Red) | ftxui::bold;
                    } else if (c == '^' || c == 'v' || c == '>' || c == '<') {
                        // 出口 - 白色文字配紫色背景
                        char_element = char_element | ftxui::color(ftxui::Color::White) | ftxui::bgcolor(ftxui::Color::Magenta) | ftxui::bold;
                    } else if (c == '#') {
                        // 墙壁 - 灰色
                        char_element = char_element | ftxui::color(ftxui::Color::GrayLight) | ftxui::bold;
                    } else if (c == '.') {
                        // 空地 - 白色
                        char_element = char_element | ftxui::color(ftxui::Color::White);
                    } else {
                        // 其他字符 - 青色
                        char_element = char_element | ftxui::color(ftxui::Color::Cyan) | ftxui::bold;
                    }
                    
                    styled_chars.push_back(char_element);
                }
                
                // 将样式化的字符组合成一行
                auto styled_line = ftxui::hbox(styled_chars);
                map_content.push_back(styled_line);
            } else if (line.find("=== 地图图例 ===") != std::string::npos) {
                // 图例标题 - 使用黄色
                auto styled_line = ftxui::text(line) | ftxui::color(ftxui::Color::Yellow) | ftxui::bold;
                map_content.push_back(styled_line);
            } else if (line.find("P =") != std::string::npos || line.find("I =") != std::string::npos || 
                      line.find("N =") != std::string::npos || line.find("S =") != std::string::npos || 
                      line.find("M =") != std::string::npos || line.find("^v>< =") != std::string::npos || 
                      line.find("# =") != std::string::npos || line.find(". =") != std::string::npos) {
                // 图例说明 - 使用普通颜色，不高亮
                auto styled_line = ftxui::text(line) | ftxui::color(ftxui::Color::GrayLight);
                map_content.push_back(styled_line);
            } else {
                // 其他内容 - 使用绿色
                auto styled_line = ftxui::text(line) | ftxui::color(ftxui::Color::Green);
                map_content.push_back(styled_line);
            }
        }
        
        // 将地图内容放在一个带边框的容器中
        auto map_box = ftxui::vbox(map_content) | ftxui::border | ftxui::color(ftxui::Color::Green);
        left.push_back(map_box);
        
        // 区块信息 - 使用更美观的样式
        if (!current_block_info_.empty()) {
            left.push_back(ftxui::separator());
            left.push_back(ftxui::text("[位置] 位置信息") | ftxui::bold | ftxui::color(ftxui::Color::Yellow));
            left.push_back(ftxui::paragraph(current_block_info_) | ftxui::color(ftxui::Color::White));
        }
        
        auto left_box = ftxui::vbox(left) | ftxui::border | ftxui::color(ftxui::Color::Blue);

        // 右侧：游戏消息（固定高度视口） + 游戏状态（玩家/队伍） - 优化美观度
        std::vector<ftxui::Element> right;
        
        // 游戏消息区域
        right.push_back(ftxui::text("[消息] 游戏消息") | ftxui::bold | ftxui::color(ftxui::Color::Magenta));
        right.push_back(ftxui::separator());
        std::vector<ftxui::Element> msg_lines;
        if (game_messages_.empty()) {
            msg_lines.push_back(ftxui::text("暂无消息") | ftxui::color(ftxui::Color::GrayLight));
        } else {
            for (const auto& m : game_messages_) {
                // 为不同类型的消息添加不同颜色
                auto styled_msg = ftxui::paragraph(m);
                if (m.find("胜利") != std::string::npos || m.find("成功") != std::string::npos) {
                    styled_msg = styled_msg | ftxui::color(ftxui::Color::Green);
                } else if (m.find("失败") != std::string::npos || m.find("错误") != std::string::npos) {
                    styled_msg = styled_msg | ftxui::color(ftxui::Color::Red);
                } else if (m.find("获得") != std::string::npos || m.find("奖励") != std::string::npos) {
                    styled_msg = styled_msg | ftxui::color(ftxui::Color::Yellow);
                } else {
                    styled_msg = styled_msg | ftxui::color(ftxui::Color::White);
                }
                msg_lines.push_back(styled_msg);
            }
        }
        auto messages_view = ftxui::vbox(msg_lines) | ftxui::vscroll_indicator | ftxui::yframe |
                             ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, message_h);
        right.push_back(messages_view);
        
        // 游戏状态区域
        right.push_back(ftxui::separator());
        right.push_back(ftxui::text("[状态] 游戏状态") | ftxui::bold | ftxui::color(ftxui::Color::Cyan));
        right.push_back(ftxui::separator());
        
        // 玩家信息 - 使用更美观的显示
        right.push_back(ftxui::paragraph("玩家: " + player_name_) | ftxui::color(ftxui::Color::White));
        right.push_back(ftxui::paragraph("等级: Lv." + std::to_string(player_level_)) | ftxui::color(ftxui::Color::Yellow));
        
        // HP显示 - 使用颜色表示血量状态
        auto hp_color = (player_hp_ > player_max_hp_ * 0.5) ? ftxui::Color::Green : 
                       (player_hp_ > player_max_hp_ * 0.25) ? ftxui::Color::Yellow : ftxui::Color::Red;
        right.push_back(ftxui::paragraph("HP: " + std::to_string(player_hp_) + "/" + std::to_string(player_max_hp_)) | ftxui::color(hp_color));
        right.push_back(ftxui::paragraph(player_status_) | ftxui::color(ftxui::Color::GrayLight));
        
        // 队伍成员区域
        right.push_back(ftxui::separator());
        right.push_back(ftxui::text("[队伍] 队伍成员") | ftxui::bold | ftxui::color(ftxui::Color::Blue));
        right.push_back(ftxui::separator());
        if (team_members_.empty()) {
            right.push_back(ftxui::text("暂无队伍成员") | ftxui::color(ftxui::Color::GrayLight));
        } else {
            for (const auto& member : team_members_) {
                // 为队伍成员添加颜色
                right.push_back(ftxui::paragraph("* " + member) | ftxui::color(ftxui::Color::White));
            }
        }
        
        auto right_box = ftxui::vbox(right) | ftxui::border | ftxui::color(ftxui::Color::Magenta);

        // 按 7:3 比例分配宽度
        int left_w = std::max(20, dims.dimx * 7 / 10);
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
        
        // 操作提示 - 使用更美观的样式
        auto help = ftxui::text("W/A/S/D 移动  空格 交互  T 工具菜单  Q/E 切换队友") | 
                   ftxui::color(ftxui::Color::GrayLight);
        
        auto bar = ftxui::hbox({ ftxui::hbox(btns), ftxui::filler(), help }) | 
                  ftxui::border | ftxui::color(ftxui::Color::Green);
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
            
            // 工具菜单标题 - 使用更美观的样式
            elements.push_back(ftxui::text("=== 工具菜单 ===") | ftxui::bold | ftxui::center | ftxui::color(ftxui::Color::Cyan));
            elements.push_back(ftxui::separator());
            
            // 工具选项 - 添加更好的样式
            for (size_t i = 0; i < tool_options_.size(); ++i) {
                auto text = tool_options_[i];
                if (i == selected_tool_button_) {
                    text = "> " + text;
                    elements.push_back(ftxui::text(text) | ftxui::color(ftxui::Color::Yellow) | ftxui::bold);
                } else {
                    text = "  " + text;
                    elements.push_back(ftxui::text(text) | ftxui::color(ftxui::Color::White));
                }
            }
            
            elements.push_back(ftxui::separator());
            elements.push_back(ftxui::text("按 Enter 选择，ESC 取消") | ftxui::color(ftxui::Color::GrayLight) | ftxui::center);
            
            return ftxui::vbox(elements) | ftxui::border | ftxui::center | ftxui::color(ftxui::Color::Blue);
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
