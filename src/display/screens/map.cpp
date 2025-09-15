#include "map.hpp"
#include "../../core/game.h"
#include "../../core/map_v2.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <sstream>
#include <algorithm>

MapScreen::MapScreen(Game* game) : game_(game), player_x_(0), player_y_(0) {
    // 初始化默认值
    player_name_ = "未知玩家";
    player_status_ = "状态未知";
    current_block_info_ = "位置信息加载中...";
    
    // 初始化地图显示
    map_lines_.resize(MAP_HEIGHT, std::string(MAP_WIDTH * 2, ' '));
    
    // 如果游戏对象存在，立即更新数据
    if (game_) {
        UpdateMapData(*game_);
    }
    
    // 创建UI组件
    map_display_ = ftxui::Renderer([this] {
        std::vector<ftxui::Element> left;

        // 地图标题（保持为“全部地图”）
        left.push_back(ftxui::text("=== 全部地图 ===") | ftxui::bold | ftxui::center | ftxui::color(ftxui::Color::Cyan));
        left.push_back(ftxui::separator());

        // 可选：当前位置/区块名（如果有）
        std::string current_block_name = "";
        if (game_) {
            auto current_block = game_->getMapManager().getCurrentBlock();
            if (current_block) {
                current_block_name = current_block->getName();
            }
        }
        if (!current_block_name.empty()) {
            left.push_back(ftxui::text("当前位置: " + current_block_name) | ftxui::color(ftxui::Color::Green));
            left.push_back(ftxui::separator());
        }

        // 地图内容 - 使用等宽布局与元素着色
        std::vector<ftxui::Element> map_content;
        for (const auto& line : map_lines_) {
            // 如果包含非 ASCII（如中文区块名），整行渲染以避免字符宽度导致的错位
            bool has_non_ascii = std::any_of(line.begin(), line.end(), [](char ch) {
                return static_cast<unsigned char>(ch) >= 128;
            });

            bool is_map_grid = line.find("=== ") == std::string::npos &&
                               line.find(": ") == std::string::npos &&
                               line.find("P =") == std::string::npos &&
                               line.find("I =") == std::string::npos &&
                               line.find("N =") == std::string::npos &&
                               line.find("S =") == std::string::npos &&
                               line.find("M =") == std::string::npos &&
                               line.find("^v>< =") == std::string::npos &&
                               line.find("# =") == std::string::npos &&
                               line.find(". =") == std::string::npos &&
                               line.length() > 5;

            if (has_non_ascii) {
                // 整行渲染，保持对齐
                map_content.push_back(ftxui::text(line));
            } else if (is_map_grid) {
                std::vector<ftxui::Element> styled_chars;
                styled_chars.reserve(line.size());
                for (size_t i = 0; i < line.length(); ++i) {
                    char c = line[i];
                    ftxui::Element ch = ftxui::text(std::string(1, c));
                    if (c == '+' || c == '-' || c == '|') {
                        ch = ch | ftxui::color(ftxui::Color::Cyan) | ftxui::bold;
                    } else if (c == 'P') {
                        ch = ch | ftxui::color(ftxui::Color::White) | ftxui::bgcolor(ftxui::Color::Blue) | ftxui::bold;
                    } else if (c == 'I') {
                        ch = ch | ftxui::color(ftxui::Color::White) | ftxui::bgcolor(ftxui::Color::Green) | ftxui::bold;
                    } else if (c == 'N') {
                        ch = ch | ftxui::color(ftxui::Color::Black) | ftxui::bgcolor(ftxui::Color::Yellow) | ftxui::bold;
                    } else if (c == 'S') {
                        ch = ch | ftxui::color(ftxui::Color::White) | ftxui::bgcolor(ftxui::Color::Magenta);
                    } else if (c == 'M') {
                        ch = ch | ftxui::color(ftxui::Color::White) | ftxui::bgcolor(ftxui::Color::Red) | ftxui::bold;
                    } else if (c == '^' || c == 'v' || c == '<' || c == '>') {
                        ch = ch | ftxui::color(ftxui::Color::Yellow) | ftxui::bold;
                    } else if (c == '#') {
                        ch = ch | ftxui::color(ftxui::Color::GrayDark);
                    } else if (c == '.') {
                        ch = ch | ftxui::color(ftxui::Color::GrayLight);
                    }
                    styled_chars.push_back(ch);
                }
                map_content.push_back(ftxui::hbox(styled_chars));
            } else if (line.find("=== 地图图例 ===") != std::string::npos) {
                map_content.push_back(ftxui::text(line) | ftxui::color(ftxui::Color::Yellow) | ftxui::bold);
            } else if (line.find("P =") != std::string::npos || line.find("I =") != std::string::npos ||
                       line.find("N =") != std::string::npos || line.find("S =") != std::string::npos ||
                       line.find("M =") != std::string::npos || line.find("^v>< =") != std::string::npos ||
                       line.find("# =") != std::string::npos || line.find(". =") != std::string::npos) {
                map_content.push_back(ftxui::text(line) | ftxui::color(ftxui::Color::GrayLight));
            } else {
                map_content.push_back(ftxui::text(line) | ftxui::color(ftxui::Color::Green));
            }
        }

        auto map_box = ftxui::vbox(map_content) | ftxui::border | ftxui::color(ftxui::Color::Green);
        left.push_back(map_box);

        // 区块信息
        if (!current_block_info_.empty()) {
            left.push_back(ftxui::separator());
            left.push_back(ftxui::text("[位置] 位置信息") | ftxui::bold | ftxui::color(ftxui::Color::Yellow));
            left.push_back(ftxui::paragraph(current_block_info_) | ftxui::color(ftxui::Color::White));
        }

        return ftxui::vbox(left) | ftxui::border | ftxui::color(ftxui::Color::Blue);
    });
    
    player_info_ = ftxui::Renderer([this] {
        std::vector<ftxui::Element> elements;

        elements.push_back(ftxui::text("玩家信息") | ftxui::bold | ftxui::color(ftxui::Color::Cyan));
        elements.push_back(ftxui::separator());
        elements.push_back(ftxui::text("姓名: " + player_name_));
        elements.push_back(ftxui::text("位置: (" + std::to_string(player_x_) + ", " + std::to_string(player_y_) + ")"));
        elements.push_back(ftxui::text("状态: " + player_status_));

        return ftxui::vbox(elements) | ftxui::border | ftxui::color(ftxui::Color::Yellow);
    });
    
    // 交互菜单
    interaction_menu_ = ftxui::Renderer([this] {
        if (!show_interaction_menu_) {
            return ftxui::text("");
        }
        
        std::vector<ftxui::Element> elements;
        elements.push_back(ftxui::text("=== 交互选项 ===") | ftxui::bold);
        
        for (size_t i = 0; i < interaction_options_.size(); ++i) {
            auto text = interaction_options_[i];
            if (i == selected_interaction_) {
                text = "> " + text;
                elements.push_back(ftxui::text(text) | ftxui::color(ftxui::Color::Yellow));
            } else {
                text = "  " + text;
                elements.push_back(ftxui::text(text));
            }
        }
        
        elements.push_back(ftxui::separator());
        elements.push_back(ftxui::text("按 Enter 选择，ESC 取消"));
        
        return ftxui::vbox(elements) | ftxui::border;
    });
    
    // 关闭按钮
    close_button_ = ftxui::Button("返回游戏", [this] {
        if (navigation_callback_) {
            navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Gameplay"));
        }
    });
    
    // 创建主组件
    std::vector<ftxui::Component> main_components;
    
    // 左侧组件
    std::vector<ftxui::Component> left_components;
    left_components.push_back(map_display_);
    
    // 只保留地图显示
    
    // 右侧组件
    std::vector<ftxui::Component> right_components;
    right_components.push_back(player_info_);
    right_components.push_back(close_button_);
    
    // 主布局
    std::vector<ftxui::Component> horizontal_components;
    horizontal_components.push_back(ftxui::Container::Vertical(left_components));
    horizontal_components.push_back(ftxui::Container::Vertical(right_components));
    
    component_ = ftxui::Container::Horizontal(horizontal_components);
    
    // 设置键盘事件处理
    // 去除交互与移动，保持只读
    component_ |= ftxui::CatchEvent([this](ftxui::Event) {
        return false;
    });
}

ftxui::Component MapScreen::GetComponent() {
    return component_;
}

void MapScreen::UpdateMapData(const Game& game) {
    try {
        // 更新玩家信息
        const auto& player = game.getPlayer();
        player_name_ = player.name;
        player_x_ = player.x;
        player_y_ = player.y;
        
        // 更新地图显示
        const auto& mapManager = game.getMapManager();
        std::vector<std::string> newMapLines;
        
        // 只显示全部地图（拼接名称视图）
        newMapLines = mapManager.renderStitchedBlocks(1);
        
        // 确保地图数据有效
        if (!newMapLines.empty()) {
            map_lines_ = newMapLines;
        } else {
            map_lines_ = {"地图数据加载失败"};
        }
        
        // 更新当前区块信息
        std::string cellInfo = mapManager.getCurrentCellInfo();
        std::string blockInfo = mapManager.getBlockInfo();
        current_block_info_ = cellInfo + "\n" + blockInfo;
        
        // 更新玩家状态
        std::stringstream ss;
        ss << "等级: " << player.level << " 经验: " << player.experience;
        if (!player.teamMembers.empty()) {
            ss << " 队伍: " << player.teamMembers.size() << "人";
        }
        player_status_ = ss.str();
        
    } catch (const std::exception& e) {
        // 错误处理
        player_name_ = "错误";
        player_status_ = "数据更新失败: " + std::string(e.what());
        current_block_info_ = "无法获取位置信息";
        map_lines_ = {"地图加载错误"};
    }
}

void MapScreen::AddMapMessage(const std::string& message) {
    map_messages_.push_back(message);
    // 限制消息数量
    if (map_messages_.size() > 10) {
        map_messages_.erase(map_messages_.begin());
    }
}

void MapScreen::ClearMapMessages() {
    map_messages_.clear();
}

void MapScreen::HandleMovement(int deltaX, int deltaY) {
    if (!game_) {
        AddMapMessage("游戏对象未初始化");
        return;
    }
    
    if (game_->movePlayer(deltaX, deltaY)) {
        auto pos = game_->getMapManager().getPlayerPosition();
        player_x_ = pos.first;
        player_y_ = pos.second;
        AddMapMessage("移动到位置 (" + std::to_string(player_x_) + ", " + std::to_string(player_y_) + ")");
    } else {
        AddMapMessage("无法移动到该位置");
    }
}

void MapScreen::HandleInteraction() {
    if (!game_) {
        AddMapMessage("游戏对象未初始化");
        return;
    }
    
    // 获取可用的交互选项
    auto interactions = game_->getAvailableMapInteractions();
    interaction_options_.clear();
    
    for (const auto& interaction : interactions) {
        switch (interaction) {
            case InteractionType::PICKUP:
                interaction_options_.push_back("拾取物品");
                break;
            case InteractionType::BATTLE:
                interaction_options_.push_back("战斗");
                break;
            case InteractionType::DIALOGUE:
                interaction_options_.push_back("对话");
                break;
            case InteractionType::ACTIVATE:
                interaction_options_.push_back("激活");
                break;
            case InteractionType::ENTER:
                interaction_options_.push_back("进入");
                break;
        }
    }
    
    if (interaction_options_.empty()) {
        AddMapMessage("当前位置没有可交互的内容");
        return;
    }
    
    selected_interaction_ = 0;
    ShowInteractionMenu();
}

void MapScreen::ShowInteractionMenu() {
    show_interaction_menu_ = true;
}

void MapScreen::HideInteractionMenu() {
    show_interaction_menu_ = false;
}

void MapScreen::HandleInteractionOption(int optionIndex) {
    if (!game_ || optionIndex < 0 || optionIndex >= interaction_options_.size()) {
        HideInteractionMenu();
        return;
    }
    
    std::string option = interaction_options_[optionIndex];
    AddMapMessage("选择了交互: " + option);
    
    // 获取对应的交互类型
    auto interactions = game_->getAvailableMapInteractions();
    if (optionIndex < interactions.size()) {
        InteractionType interactionType = interactions[optionIndex];
        auto result = game_->interactWithMap(interactionType);
        
        AddMapMessage(result.message);
        if (result.success) {
            // 更新地图显示
            UpdateMapData(*game_);
        }
    }
    
    HideInteractionMenu();
}

void MapScreen::UpdateMapDisplay() {
    // 强制更新地图显示
    // 这个方法可以在需要时被调用来刷新显示
}

void MapScreen::RefreshMapDisplay() {
    if (game_) {
        try {
            UpdateMapData(*game_);
            AddMapMessage("地图数据已刷新");
        } catch (const std::exception& e) {
            AddMapMessage("地图刷新失败: " + std::string(e.what()));
        }
    } else {
        AddMapMessage("无法刷新地图：游戏对象未初始化");
    }
}

void MapScreen::ToggleMapView() {
    // 已移除视图切换，保持为完整地图视图
    RefreshMapDisplay();
}
