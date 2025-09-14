//
// Created by Assistant on 2025/1/1.
//

#ifndef CPP_MUD_OUC_GAMEPLAY_HPP
#define CPP_MUD_OUC_GAMEPLAY_HPP
#include "../display.hpp"
#include "../../core/gamestate.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <string>
#include <vector>
#include <memory>

// 前向声明
class Player;
class MapManager;

class GameplayScreen : public BaseScreen {
public:
    GameplayScreen(Game* game = nullptr);
    ~GameplayScreen() = default;

    // 实现基类的虚函数
    ftxui::Component GetComponent() override;

    // 游戏状态更新方法
    void UpdatePlayerInfo(const Player& player);
    void AddChatMessage(const std::string& message);
    void UpdateGameStatus(const std::string& status);
    void UpdateTeamStatus(const std::vector<std::string>& teamMembers);
    void UpdateMapDisplay();
    
    // 辅助方法：刷新队伍状态显示
    void RefreshTeamDisplay();
    
    // 清空游戏消息和聊天消息
    void ClearAllMessages();

private:
    void HandleGameCommand(const std::string& command);
    ftxui::Component component_;
    
    // UI组件
    ftxui::Component chat_input_;
    ftxui::Component game_input_;
    std::vector<ftxui::Component> bottom_action_buttons_; // 底部操作按钮（鼠标点击）

    // 数据存储
    std::vector<std::string> chat_messages_;
    std::vector<std::string> game_messages_;

    // 游戏状态
    std::string player_name_;
    int player_hp_ = 0;
    int player_max_hp_ = 0;
    int player_level_ = 0;
    int player_experience_ = 0;
    std::string player_status_;
    std::vector<std::string> team_members_;
    
    // 地图显示
    std::vector<std::string> current_map_lines_;
    std::string current_block_info_;
    
    // 输入缓冲区
    std::string chat_input_buffer_;
    
    // 命令选项
    std::vector<std::string> command_options_;  // 游戏命令选项列表
    
    // 游戏对象引用
    Game* game_;
};

#endif //CPP_MUD_OUC_GAMEPLAY_HPP
