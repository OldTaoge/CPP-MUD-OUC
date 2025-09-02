#include "questscreen.hpp"
#include "../../player/player.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/event.hpp>
#include <algorithm>
#include <iostream>

using namespace ftxui;

QuestScreen::QuestScreen(Player* player) : player_(player) {
    // 创建返回按钮
    back_button_ = Button("返回游戏", [this] {
        this->ReturnToGame();
    });
    
    // 创建任务列表组件
    quest_list_ = Container::Vertical({});
    
    // 创建任务详情相关按钮
    start_button_ = Button("开始任务", [this] {
        if (selected_quest_index_ >= 0 && selected_quest_index_ < static_cast<int>(player_->getQuests().size())) {
            this->StartQuest(selected_quest_index_);
        }
    });
    
    complete_button_ = Button("完成任务", [this] {
        if (selected_quest_index_ >= 0 && selected_quest_index_ < static_cast<int>(player_->getQuests().size())) {
            this->CompleteQuest(selected_quest_index_);
        }
    });
    
    claim_reward_button_ = Button("领取奖励", [this] {
        if (selected_quest_index_ >= 0 && selected_quest_index_ < static_cast<int>(player_->getQuests().size())) {
            this->ClaimQuestReward(selected_quest_index_);
        }
    });
    
    close_detail_button_ = Button("关闭详情", [this] {
        this->HideQuestDetails();
    });
    
    // 创建任务详情叠加层
    quest_detail_overlay_ = Container::Vertical({start_button_, complete_button_, claim_reward_button_, close_detail_button_});
    
    // 创建主组件
    component_ = Container::Vertical({back_button_, quest_list_, quest_detail_overlay_});
    
    // 更新任务列表
    UpdateQuestList();
    
    // 设置渲染器
    component_ = Renderer(component_, [this] {
        // 构建任务列表元素
        Elements quest_elements;
        for (const auto& quest : player_->getQuests()) {
            std::string questText = quest.name + " - " + quest.getStatusString();
            quest_elements.push_back(text(questText) | color(Color::White));
        }
        
        // 构建任务列表区域
        auto quest_area = vbox({
            text("任务列表") | bold | color(Color::Green) | hcenter,
            separator(),
            vbox({
                vbox(quest_elements) | flex | border
            })
        }) | flex;
        
        // 构建底部按钮区域
        auto bottom_area = hbox({
            back_button_->Render() | hcenter | flex
        });
        
        // 基础界面
        auto base_interface = vbox({
            quest_area | flex,
            separator(),
            bottom_area
        }) | border;
        
        // 任务详情叠加层
        if (show_quest_detail_ && selected_quest_index_ >= 0 && selected_quest_index_ < static_cast<int>(player_->getQuests().size())) {
            const Quest& selectedQuest = player_->getQuests()[selected_quest_index_];
            
            Elements detail_elements;
            detail_elements.push_back(text("任务详情") | bold | color(Color::Yellow) | hcenter);
            detail_elements.push_back(separator());
            detail_elements.push_back(text("名称: " + selectedQuest.name) | color(Color::White));
            detail_elements.push_back(text("状态: " + selectedQuest.getStatusString()) | color(Color::White));
            detail_elements.push_back(text("描述: " + selectedQuest.description) | color(Color::White));
            
            // 添加任务目标
            detail_elements.push_back(text("目标:") | bold | color(Color::Cyan));
            for (const auto& objective : selectedQuest.objectives) {
                detail_elements.push_back(text("  " + objective.getProgressText()) | color(Color::White));
            }
            
            // 添加任务奖励
            if (!selectedQuest.reward.empty()) {
                detail_elements.push_back(text("奖励:") | bold | color(Color::Green));
                detail_elements.push_back(text("  " + selectedQuest.reward) | color(Color::White));
                if (selectedQuest.exp_reward > 0) {
                    detail_elements.push_back(text("  经验值: +" + std::to_string(selectedQuest.exp_reward)) | color(Color::White));
                }
                if (selectedQuest.gold_reward > 0) {
                    detail_elements.push_back(text("  金币: +" + std::to_string(selectedQuest.gold_reward)) | color(Color::White));
                }
            }
            
            detail_elements.push_back(separator());
            
            // 根据任务状态添加不同的操作按钮
            if (selectedQuest.status == QuestStatus::NOT_STARTED) {
                detail_elements.push_back(start_button_->Render());
            } else if (selectedQuest.status == QuestStatus::IN_PROGRESS && selectedQuest.isCompleted()) {
                detail_elements.push_back(complete_button_->Render());
            } else if (selectedQuest.status == QuestStatus::COMPLETED) {
                detail_elements.push_back(claim_reward_button_->Render());
            }
            
            detail_elements.push_back(close_detail_button_->Render() | hcenter);
            
            auto overlay_element = vbox(detail_elements) | border | bgcolor(Color::DarkBlue) | color(Color::White);
            
            return overlay_element | hcenter | vcenter;
        } else {
            return base_interface;
        }
    });
}

Component QuestScreen::GetComponent() {
    return component_;
}

void QuestScreen::UpdateQuestList() {
    // 清空现有任务按钮
    quest_buttons_.clear();
    quest_list_->DetachAllChildren();
    
    // 为每个任务创建按钮
    for (size_t i = 0; i < player_->getQuests().size(); ++i) {
        auto button = Button(player_->getQuests()[i].name, [this, i] { 
            this->ShowQuestDetails(i); 
        });
        quest_buttons_.push_back(button);
        quest_list_->Add(button);
    }
}

void QuestScreen::HandleQuestAction(int questIndex, const std::string& action) {
    if (questIndex < 0 || questIndex >= static_cast<int>(player_->getQuests().size())) {
        return;
    }
    
    const Quest& quest = player_->getQuests()[questIndex];
    if (action == "start" && quest.status == QuestStatus::NOT_STARTED) {
        StartQuest(questIndex);
    } else if (action == "complete" && quest.status == QuestStatus::IN_PROGRESS && quest.isCompleted()) {
        CompleteQuest(questIndex);
    } else if (action == "claim" && quest.status == QuestStatus::COMPLETED) {
        ClaimQuestReward(questIndex);
    }
}

void QuestScreen::ShowQuestDetails(int questIndex) {
    selected_quest_index_ = questIndex;
    show_quest_detail_ = true;
}

void QuestScreen::HideQuestDetails() {
    show_quest_detail_ = false;
    selected_quest_index_ = -1;
}

void QuestScreen::StartQuest(int questIndex) {
    if (questIndex < 0 || questIndex >= static_cast<int>(player_->getQuests().size())) {
        return;
    }
    
    const Quest& quest = player_->getQuests()[questIndex];
    if (player_->startQuest(quest.id)) {
        UpdateQuestList();
        HideQuestDetails();
    }
}

void QuestScreen::CompleteQuest(int questIndex) {
    if (questIndex < 0 || questIndex >= static_cast<int>(player_->getQuests().size())) {
        return;
    }
    
    const Quest& quest = player_->getQuests()[questIndex];
    if (player_->completeQuest(quest.id)) {
        UpdateQuestList();
        HideQuestDetails();
    }
}

void QuestScreen::ClaimQuestReward(int questIndex) {
    if (questIndex < 0 || questIndex >= static_cast<int>(player_->getQuests().size())) {
        return;
    }
    
    const Quest& quest = player_->getQuests()[questIndex];
    if (player_->claimQuestReward(quest.id)) {
        UpdateQuestList();
        HideQuestDetails();
    }
}

void QuestScreen::ReturnToGame() {
    // 切换回游戏界面
    if (navigation_callback_) {
        navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Gameplay"));
    }
}