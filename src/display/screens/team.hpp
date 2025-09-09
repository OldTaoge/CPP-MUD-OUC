#pragma once
#include "../display.hpp"
#include <ftxui/component/component.hpp>
#include <string>
#include <vector>
#include <memory>

// 前向声明
class Game;
class TeamMember;

class TeamScreen : public BaseScreen {
public:
    TeamScreen(Game* game = nullptr);
    ~TeamScreen() = default;

    // 实现基类的虚函数
    ftxui::Component GetComponent() override;

    // 对外提供刷新方法（用于切换到该界面时同步最新队伍数据）
    void Refresh();

private:
    void RefreshTeamData();
    void HandleMemberToggle(int index);
    void HandleAddMember();
    void HandleEquipMember(int memberIndex);
    void HandleSwitchToMember(int memberIndex);
    
    // UI组件
    ftxui::Component component_;
    std::vector<ftxui::Component> member_toggle_buttons_;
    std::vector<ftxui::Component> member_equip_buttons_;
    std::vector<ftxui::Component> member_switch_buttons_;
    ftxui::Component add_member_button_;
    ftxui::Component back_button_;
    
    // 输入组件
    ftxui::Component new_member_name_input_;
    std::string new_member_name_buffer_;
    
    // 数据存储
    struct MemberDisplayInfo {
        std::string name;
        int level;
        int health;
        int maxHealth;
        bool isActive;
        bool isCurrentActive;
        std::string weapon;
        std::string artifact;
        std::string status;
    };
    
    std::vector<MemberDisplayInfo> member_info_;
    int selected_member_ = 0;
    bool show_add_member_dialog_ = false;
    
    // 游戏对象引用
    Game* game_;
    
    // 状态信息
    std::string status_message_;
    int active_count_;
    int max_active_count_;
};
