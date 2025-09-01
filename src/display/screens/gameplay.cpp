//
// Created by Assistant on 2025/1/1.
//

#include "gameplay.hpp"
#include "../player/player.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/event.hpp>
#include <algorithm>
#include <iostream>

using namespace ftxui;

GameplayScreen::GameplayScreen() {
    // 初始化工具选项
    tool_options_ = {
        "背包 - 查看和管理物品",
        "地图 - 查看世界地图和传送点", 
        "任务 - 查看当前任务和成就",
        "设置 - 调整游戏设置",
        "返回 - 保存游戏或退出"
    };
    
    // 初始化游戏状态
    player_name_ = "旅行者";
    player_hp_ = 100;
    player_max_hp_ = 100;
    player_status_ = "正常";
    team_members_ = {"派蒙", "温迪", "钟离"};
    
    // 添加一些初始消息
    AddChatMessage("欢迎来到《原神》世界！我是派蒙，你的向导。", true);
    AddChatMessage("有什么需要帮助的吗？输入帮助查看命令。");
    UpdateGameStatus("你站在蒙德城的广场上，周围是熙熙攘攘的人群。");
    
    // 创建聊天输入组件
    chat_input_ = Input(&chat_input_buffer_, "与派蒙对话...");
    
    // 创建游戏命令输入组件
    game_input_ = Input(&game_input_buffer_, "输入游戏命令...");
    
    // 创建单一工具按钮
    tool_button_ = Button("工具", [this] {
        this->ShowToolOverlay();
    });
    
    // 创建工具叠加图层（初始为空）
    tool_overlay_ = Container::Vertical({});
    
    // 创建主组件
    component_ = Container::Vertical({
        chat_input_,
        game_input_,
        tool_button_,
        tool_overlay_
    });
    
    // 设置渲染器
    component_ = Renderer(component_, [this] {
        // 检查输入缓冲区变化并处理
        static std::string last_chat_input = "";
        static std::string last_game_input = "";
        
        if (chat_input_buffer_ != last_chat_input && !chat_input_buffer_.empty() && 
            chat_input_buffer_.find('\n') != std::string::npos) {
            // 聊天输入完成
            std::string input = chat_input_buffer_;
            input.erase(std::remove(input.begin(), input.end(), '\n'), input.end());
            if (!input.empty()) {
                AddChatMessage("你: " + input);
                AddChatMessage("派蒙: 我明白了！让我想想...", true);
            }
            chat_input_buffer_.clear();
        }
        
        if (game_input_buffer_ != last_game_input && !game_input_buffer_.empty() && 
            game_input_buffer_.find('\n') != std::string::npos) {
            // 游戏命令输入完成
            std::string input = game_input_buffer_;
            input.erase(std::remove(input.begin(), input.end(), '\n'), input.end());
            if (!input.empty()) {
                HandleGameCommand(input);
            }
            game_input_buffer_.clear();
        }
        
        last_chat_input = chat_input_buffer_;
        last_game_input = game_input_buffer_;
        
        // 构建聊天消息元素
        Elements chat_elements;
        for (const auto& msg : chat_messages_) {
            chat_elements.push_back(text(msg) | color(Color::White));
        }
        
        // 构建游戏消息元素
        Elements game_elements;
        for (const auto& msg : game_messages_) {
            game_elements.push_back(text(msg) | color(Color::White));
        }
        
        // 构建队伍成员元素
        Elements team_elements;
        for (const auto& member : team_members_) {
            team_elements.push_back(text("• " + member) | color(Color::Blue));
        }
        
        // 顶部：LLM对话框 + 右侧工具按钮
        auto chat_area = vbox({
            hbox({
                text("派蒙") | bold | color(Color::Yellow),
                text("《原神》MUD版") | bold | color(Color::Cyan) | hcenter | flex,
                tool_button_->Render()
            }),
            separator(),
            vbox({
                vbox(chat_elements) | flex | border,
                hbox({
                    text("你: ") | color(Color::Cyan),
                    chat_input_->Render() | flex
                })
            }) | flex
        }) | size(HEIGHT, EQUAL, 15);  // 固定高度为15行
        /*
        auto tool_area = vbox({
            text("工具") | bold | color(Color::Blue),
            separator(),
            hbox({
                text("工具") | bold | color(Color::Blue),
                tool_button_->Render() | hcenter
            }) | hcenter
        }) | size(WIDTH, LESS_THAN, 25);
        */
        auto top_row = hbox({
            chat_area | flex,
            separator(),
           // tool_area
        });
        
        // 中间：游戏交互主界面
        auto game_area = vbox({
            text("游戏世界") | bold | color(Color::Green),
            separator(),
            vbox({
                vbox(game_elements) | flex | border,
                hbox({
                    text("命令: ") | color(Color::Red),
                    game_input_->Render() | flex
                })
            }) | flex
        }) | size(HEIGHT, EQUAL, 20);  // 固定高度为20行
        
        // 底部：人物HP + 人物状态 + 队伍状态
        auto status_area = hbox({
            vbox({
                text("生命值") | bold | color(Color::Red),
                text(player_name_ + ": " + std::to_string(player_hp_) + "/" + std::to_string(player_max_hp_)) | color(Color::Red)
            }) | border | flex,
            separator(),
            vbox({
                text("状态") | bold | color(Color::Yellow),
                text(player_status_) | color(Color::Yellow)
            }) | border | flex,
            separator(),
            vbox({
                text("队伍") | bold | color(Color::Blue),
                vbox(team_elements)
            }) | border | flex
        }) | size(HEIGHT, EQUAL, 8);  // 固定高度为8行
        
        // 基础界面
        auto base_interface = vbox({
            top_row | flex,
            separator(),
            game_area | flex,
            separator(),
            status_area
        }) | border;
        
        // 工具叠加图层
        if (show_tool_overlay_) {
            Elements overlay_options;
            for (size_t i = 0; i < tool_options_.size(); ++i) {
                auto option_button = Button(tool_options_[i], [this, i] {
                    this->HandleToolOption(i);
                });
                overlay_options.push_back(option_button->Render());
            }
            
            auto close_button = Button("关闭", [this] {
                this->HideToolOverlay();
            });
            
            auto overlay_element = vbox({
                text("工具菜单") | bold | color(Color::Magenta) | hcenter,
                separator(),
                vbox(overlay_options) | border,
                separator(),
                close_button->Render() | hcenter
            }) | border | bgcolor(Color::DarkBlue) | color(Color::White);
            
            return overlay_element | hcenter | vcenter;
        } else {
            return base_interface;
        }
    });
}

Component GameplayScreen::GetComponent() {
    return component_;
}

void GameplayScreen::UpdatePlayerInfo(const Player& player) {
    player_name_ = player.name;
    player_hp_ = player.health;
    // 这里可以添加更多玩家信息的更新
}

void GameplayScreen::AddChatMessage(const std::string& message, bool isLLM) {
    chat_messages_.push_back(message);
    // 限制消息数量，避免内存占用过多
    if (chat_messages_.size() > 50) {
        chat_messages_.erase(chat_messages_.begin());
    }
}

void GameplayScreen::UpdateGameStatus(const std::string& status) {
    game_messages_.clear();
    game_messages_.push_back(status);
}

void GameplayScreen::UpdateTeamStatus(const std::vector<std::string>& teamMembers) {
    team_members_ = teamMembers;
}

void GameplayScreen::HandleToolButton(int buttonIndex) {
    switch (buttonIndex) {
        case 0: // 背包
            AddChatMessage("派蒙: 打开背包查看物品", true);
            break;
        case 1: // 地图
            AddChatMessage("派蒙: 显示世界地图", true);
            break;
        case 2: // 任务
            AddChatMessage("派蒙: 查看当前任务", true);
            break;
        case 3: // 设置
            if (navigation_callback_) {
                navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Settings"));
            }
            break;
        case 4: // 返回
            if (navigation_callback_) {
                navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "MainMenu"));
            }
            break;
    }
}

void GameplayScreen::HandleGameCommand(const std::string& command) {
    std::string lowerCommand = command;
    std::transform(lowerCommand.begin(), lowerCommand.end(), lowerCommand.begin(), ::tolower);
    
    if (lowerCommand.find("移动") != std::string::npos || lowerCommand.find("走") != std::string::npos) {
        UpdateGameStatus("你开始移动...");
        AddChatMessage("派蒙: 好的，我们走吧！", true);
    } else if (lowerCommand.find("攻击") != std::string::npos || lowerCommand.find("战斗") != std::string::npos) {
        UpdateGameStatus("你进入战斗状态！");
        AddChatMessage("派蒙: 小心！敌人出现了！", true);
    } else if (lowerCommand.find("查看") != std::string::npos || lowerCommand.find("观察") != std::string::npos) {
        UpdateGameStatus("你仔细观察周围的环境...");
        AddChatMessage("派蒙: 这里有很多有趣的东西呢！", true);
    } else if (lowerCommand.find("帮助") != std::string::npos) {
        AddChatMessage("派蒙: 你可以尝试输入：移动、攻击、查看、帮助等命令", true);
    } else {
        UpdateGameStatus("你尝试了 '" + command + "'");
        AddChatMessage("派蒙: 我不太明白你的意思，试试其他命令吧！", true);
    }
}

void GameplayScreen::ShowToolOverlay() {
    show_tool_overlay_ = true;
}

void GameplayScreen::HideToolOverlay() {
    show_tool_overlay_ = false;
}

void GameplayScreen::HandleToolOption(int optionIndex) {
    HideToolOverlay();
    
    switch (optionIndex) {
        case 0: // 背包
            AddChatMessage("派蒙: 打开背包查看物品", true);
            break;
        case 1: // 地图
            AddChatMessage("派蒙: 显示世界地图", true);
            break;
        case 2: // 任务
            AddChatMessage("派蒙: 查看当前任务", true);
            break;
        case 3: // 设置
            if (navigation_callback_) {
                navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Settings"));
            }
            break;
        case 4: // 返回
            if (navigation_callback_) {
                navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "MainMenu"));
            }
            break;
    }
}
