#include "gameplay.hpp"
#include "../../core/game.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
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
    
    // 创建主组件
    std::vector<ftxui::Component> main_components;
    
    // 左侧：游戏信息
    std::vector<ftxui::Component> left_components;
    left_components.push_back(ftxui::Renderer([this] {
        std::vector<ftxui::Element> elements;
        elements.push_back(ftxui::text("=== Game Status ===") | ftxui::bold);
        elements.push_back(ftxui::text("Player: " + player_name_));
        elements.push_back(ftxui::text("HP: " + std::to_string(player_hp_) + "/" + std::to_string(player_max_hp_)));
        elements.push_back(ftxui::text("Status: " + player_status_));
        return ftxui::vbox(elements);
    }));
    left_components.push_back(ftxui::Renderer([this] {
        std::vector<ftxui::Element> elements;
        elements.push_back(ftxui::text("=== Team Members ===") | ftxui::bold);
        for (const auto& member : team_members_) {
            elements.push_back(ftxui::text("• " + member));
        }
        return ftxui::vbox(elements);
    }));
    
    // 添加当前区块地图显示
    left_components.push_back(ftxui::Renderer([this] {
        std::vector<ftxui::Element> elements;
        elements.push_back(ftxui::text("=== Current Area Map ===") | ftxui::bold);
        
        // 显示地图
        for (const auto& line : current_map_lines_) {
            elements.push_back(ftxui::text(line));
        }
        
        // 显示当前位置信息
        if (!current_block_info_.empty()) {
            elements.push_back(ftxui::separator());
            elements.push_back(ftxui::text(current_block_info_));
        }
        
        return ftxui::vbox(elements);
    }));
    
    // 右侧：聊天和输入
    std::vector<ftxui::Component> right_components;
    right_components.push_back(ftxui::Renderer([this] {
        std::vector<ftxui::Element> elements;
        elements.push_back(ftxui::text("=== Game Messages ===") | ftxui::bold);
        for (const auto& message : game_messages_) {
            elements.push_back(ftxui::text(message));
        }
        return ftxui::vbox(elements);
    }));
    right_components.push_back(ftxui::Renderer([this] {
        std::vector<ftxui::Element> elements;
        elements.push_back(ftxui::text("=== Chat ===") | ftxui::bold);
        for (const auto& message : chat_messages_) {
            elements.push_back(ftxui::text(message));
        }
        return ftxui::vbox(elements);
    }));
    right_components.push_back(chat_input_);
    right_components.push_back(game_input_);
    
    // 水平布局
    std::vector<ftxui::Component> horizontal_components;
    horizontal_components.push_back(ftxui::Container::Vertical(left_components));
    horizontal_components.push_back(ftxui::Container::Vertical(right_components));
    
    // 底部：工具按钮
    std::vector<ftxui::Component> bottom_components;
    bottom_components.push_back(tool_button_);
    bottom_components.push_back(ftxui::Renderer([this] {
        return ftxui::text("W/A/S/D移动 空格交互 T工具菜单 Q/E切换队友");
    }));
    
    // 主组件
    main_components.push_back(ftxui::Container::Horizontal(horizontal_components));
    main_components.push_back(ftxui::Container::Horizontal(bottom_components));
    
    component_ = ftxui::Container::Vertical(main_components);
    
    // 将工具菜单集成到主组件中
    std::vector<ftxui::Component> final_components;
    final_components.push_back(component_);
    
    // 添加工具菜单作为条件组件
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
        } else {
            return ftxui::text("");
        }
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
