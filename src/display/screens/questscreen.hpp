#ifndef CPP_MUD_OUC_QUESTSCREEN_HPP
#define CPP_MUD_OUC_QUESTSCREEN_HPP
#include "../display.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <string>
#include <vector>
#include <memory>
#include "../../player/quest.h"

// 前向声明
class Player;

class QuestScreen : public BaseScreen {
public:
    QuestScreen(Player* player);
    ~QuestScreen() = default;

    // 实现基类的虚函数
    ftxui::Component GetComponent() override;
    
    // 更新任务列表
    void UpdateQuestList();
    
    // 处理任务操作
    void HandleQuestAction(int questIndex, const std::string& action);

private:
    // 显示任务详情
    void ShowQuestDetails(int questIndex);
    
    // 关闭任务详情
    void HideQuestDetails();
    
    // 玩家对象指针
    Player* player_;
    
    // 开始任务
    void StartQuest(int questIndex);
    
    // 完成任务
    void CompleteQuest(int questIndex);
    
    // 领取任务奖励
    void ClaimQuestReward(int questIndex);
    
    // 返回游戏界面
    void ReturnToGame();

    ftxui::Component component_;
    
    // UI组件
    ftxui::Component back_button_;
    ftxui::Component quest_list_;
    std::vector<ftxui::Component> quest_buttons_;
    ftxui::Component quest_detail_overlay_;
    ftxui::Component start_button_;
    ftxui::Component complete_button_;
    ftxui::Component claim_reward_button_;
    ftxui::Component close_detail_button_;
    
    // 状态
    int selected_quest_index_ = -1;
    bool show_quest_detail_ = false;
};

#endif //CPP_MUD_OUC_QUESTSCREEN_HPP