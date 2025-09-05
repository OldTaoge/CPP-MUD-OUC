//
// Created by Assistant on 2025/1/1.
//

#ifndef CPP_MUD_OUC_GAMEPLAY_HPP
#define CPP_MUD_OUC_GAMEPLAY_HPP
#include "../display.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <string>
#include <vector>
#include <memory>

// 前向声明
class Player;

class GameplayScreen : public BaseScreen {
public:
    GameplayScreen();
    ~GameplayScreen() = default;

    // 实现基类的虚函数
    ftxui::Component GetComponent() override;

    // 游戏状态更新方法
    void UpdatePlayerInfo(const Player& player);
    void AddChatMessage(const std::string& message);
    void UpdateGameStatus(const std::string& status);
    void UpdateTeamStatus(const std::vector<std::string>& teamMembers);

private:
    void HandleToolButton(int buttonIndex);
    void HandleGameCommand(const std::string& command);
    void ShowToolOverlay();
    void HideToolOverlay();
    void HandleToolOption(int optionIndex);
    
    ftxui::Component component_;
    
    // UI组件
    ftxui::Component chat_input_;
    ftxui::Component game_input_;
    ftxui::Component tool_button_;
    ftxui::Component tool_overlay_;  // 工具叠加图层
    ftxui::Component close_button_;  // 关闭按钮
    std::vector<ftxui::Component> tool_option_buttons_;  // 工具选项按钮
    
    // 数据存储
    std::vector<std::string> chat_messages_;
    std::vector<std::string> game_messages_;
    std::vector<std::string> tool_options_;  // 工具选项列表
    
    // 游戏状态
    std::string player_name_;
    int player_hp_;
    int player_max_hp_;
    std::string player_status_;
    std::vector<std::string> team_members_;
    
    // 输入缓冲区
    std::string chat_input_buffer_;
    std::string game_input_buffer_;
    
    // 选择状态
    int selected_tool_button_ = 0;
    bool show_tool_overlay_ = false;  // 是否显示工具叠加图层
};

#endif //CPP_MUD_OUC_GAMEPLAY_HPP
