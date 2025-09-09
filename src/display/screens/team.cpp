#include "team.hpp"
#include "../../core/game.h"
#include "../../player/player.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <sstream>

TeamScreen::TeamScreen(Game* game) : game_(game) {
    new_member_name_buffer_ = "";
    status_message_ = "队伍配置";
    active_count_ = 0;
    max_active_count_ = Player::MAX_ACTIVE_MEMBERS;
    
    // 创建输入组件
    new_member_name_input_ = ftxui::Input(&new_member_name_buffer_, "输入新成员姓名...");
    
    // 创建返回按钮
    back_button_ = ftxui::Button("返回游戏", [this] {
        if (navigation_callback_) {
            navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Gameplay"));
        }
    });
    
    // 初始化队伍数据
    RefreshTeamData();
    
    // 创建主组件
    component_ = ftxui::Renderer([this] {
        std::vector<ftxui::Element> elements;
        
        // 标题
        elements.push_back(ftxui::text("=== 队伍配置 ===") | ftxui::bold | ftxui::center);
        elements.push_back(ftxui::separator());
        
        // 状态信息
        std::stringstream ss;
        ss << "上场成员: " << active_count_ << "/" << max_active_count_;
        elements.push_back(ftxui::text(ss.str()) | ftxui::color(ftxui::Color::Green));
        elements.push_back(ftxui::text(status_message_));
        elements.push_back(ftxui::separator());
        
        // 成员列表
        elements.push_back(ftxui::text("队伍成员:") | ftxui::bold);
        
        if (member_info_.empty()) {
            elements.push_back(ftxui::text("暂无队伍成员") | ftxui::color(ftxui::Color::Red));
        } else {
            for (const auto & member : member_info_) {
                std::vector<ftxui::Element> member_row;
                
                // 成员基本信息
                std::stringstream member_info;
                member_info << member.name << " (Lv." << member.level << ")";
                
                ftxui::Element name_element = ftxui::text(member_info.str());
                if (member.isCurrentActive) {
                    name_element = name_element | ftxui::color(ftxui::Color::Yellow) | ftxui::bold;
                } else if (member.isActive) {
                    name_element = name_element | ftxui::color(ftxui::Color::Green);
                }
                
                member_row.push_back(name_element);
                
                // 血量信息
                std::stringstream health_info;
                health_info << "HP: " << member.health << "/" << member.maxHealth;
                member_row.push_back(ftxui::text(health_info.str()));
                
                // 状态
                ftxui::Element status_element = ftxui::text(member.status);
                if (member.isActive) {
                    status_element = status_element | ftxui::color(ftxui::Color::Green);
                } else {
                    status_element = status_element | ftxui::color(ftxui::Color::Blue);
                }
                member_row.push_back(status_element);
                
                elements.push_back(ftxui::hbox(member_row) | ftxui::border);
                
                // 装备信息
                if (!member.weapon.empty() || !member.artifact.empty()) {
                    std::vector<ftxui::Element> equip_row;
                    if (!member.weapon.empty()) {
                        equip_row.push_back(ftxui::text("武器: " + member.weapon) | ftxui::color(ftxui::Color::Cyan));
                    }
                    if (!member.artifact.empty()) {
                        equip_row.push_back(ftxui::text("圣遗物: " + member.artifact) | ftxui::color(ftxui::Color::Magenta));
                    }
                    if (!equip_row.empty()) {
                        elements.push_back(ftxui::hbox(equip_row));
                    }
                }
                
                // 操作按钮
                std::vector<ftxui::Element> button_row;
                
                // 上场/下场按钮
                std::string toggle_text = member.isActive ? "下场" : "上场";
                button_row.push_back(ftxui::text("[1] " + toggle_text));
                
                // 切换按钮（只对上场成员显示）
                if (member.isActive) {
                    button_row.push_back(ftxui::text("[2] 切换"));
                }
                
                // 装备按钮
                button_row.push_back(ftxui::text("[3] 装备"));
                
                elements.push_back(ftxui::hbox(button_row) | ftxui::color(ftxui::Color::GrayLight));
                elements.push_back(ftxui::separator());
            }
        }
        
        // 操作说明
        elements.push_back(ftxui::separator());
        elements.push_back(ftxui::text("操作说明:") | ftxui::bold);
        elements.push_back(ftxui::text("↑↓ 选择成员  [B] 返回游戏"));
        elements.push_back(ftxui::text("数字键进行对应操作"));
        
        return ftxui::vbox(elements) | ftxui::border;
    });
    
    // 添加键盘事件处理
    component_ |= ftxui::CatchEvent([this](ftxui::Event event) {
            if (event == ftxui::Event::ArrowUp) {
                selected_member_ = std::max(0, selected_member_ - 1);
                return true;
            } else if (event == ftxui::Event::ArrowDown) {
                selected_member_ = std::min((int)member_info_.size() - 1, selected_member_ + 1);
                return true;
            } else if (event == ftxui::Event::Character('b') || event == ftxui::Event::Character('B')) {
                if (navigation_callback_) {
                    navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Gameplay"));
                }
                return true;
            } else if (event == ftxui::Event::Character('1')) {
                HandleMemberToggle(selected_member_);
                return true;
            } else if (event == ftxui::Event::Character('2')) {
                HandleSwitchToMember(selected_member_);
                return true;
            } else if (event == ftxui::Event::Character('3')) {
                HandleEquipMember(selected_member_);
                return true;
            }
        return false;
    });
}

ftxui::Component TeamScreen::GetComponent() {
    return component_;
}

void TeamScreen::Refresh() {
    RefreshTeamData();
}

void TeamScreen::RefreshTeamData() {
    member_info_.clear();
    
    if (!game_) {
        status_message_ = "游戏对象未初始化";
        return;
    }
    
    const auto& player = game_->getPlayer();
    const auto& teamMembers = player.getTeamMembers();
    auto activeMember = player.getActiveMember();
    
    active_count_ = player.getActiveCount();
    
    for (const auto& member : teamMembers) {
        MemberDisplayInfo info;
        info.name = member->getName();
        info.level = member->getLevel();
        info.health = member->getCurrentHealth();
        info.maxHealth = member->getTotalHealth();
        info.isActive = member->isActive();
        info.isCurrentActive = (member == activeMember);
        
        // 装备信息
        auto weapon = member->getEquippedWeapon();
        info.weapon = weapon ? weapon->getName() : "";
        
        auto artifact = member->getEquippedArtifact();
        info.artifact = artifact ? artifact->getName() : "";
        
        // 状态信息
        switch (member->getStatus()) {
            case MemberStatus::ACTIVE:
                info.status = "上场";
                break;
            case MemberStatus::STANDBY:
                info.status = "待命";
                break;
            case MemberStatus::INJURED:
                info.status = "受伤";
                break;
        }
        
        member_info_.push_back(info);
    }
    
    status_message_ = "队伍配置 - 共" + std::to_string(teamMembers.size()) + "名成员";
}

void TeamScreen::HandleMemberToggle(int index) {
    if (!game_ || index < 0 || index >= member_info_.size()) {
        status_message_ = "无效的成员索引";
        return;
    }
    
    auto& player = game_->getPlayer();
    bool currentlyActive = member_info_[index].isActive;
    
    if (player.setMemberActive(index, !currentlyActive)) {
        RefreshTeamData();
        if (currentlyActive) {
            status_message_ = member_info_[index].name + " 已下场";
        } else {
            status_message_ = member_info_[index].name + " 已上场";
        }
    } else {
        if (!currentlyActive && active_count_ >= max_active_count_) {
            status_message_ = "上场人数已达上限 (" + std::to_string(max_active_count_) + ")";
        } else if (currentlyActive && active_count_ <= 1) {
            status_message_ = "至少需要一名成员上场";
        } else {
            status_message_ = "无法改变成员状态";
        }
    }
}

void TeamScreen::HandleAddMember() {
    if (!game_) {
        status_message_ = "游戏对象未初始化";
        return;
    }
    
    if (new_member_name_buffer_.empty()) {
        status_message_ = "请输入成员姓名";
        return;
    }
    
    // 检查重名
    const auto& teamMembers = game_->getPlayer().getTeamMembers();
    for (const auto& member : teamMembers) {
        if (member->getName() == new_member_name_buffer_) {
            status_message_ = "已存在同名成员";
            return;
        }
    }
    
    // 添加新成员
    auto& player = game_->getPlayer();
    player.addTeamMember(new_member_name_buffer_, 1);
    
    status_message_ = "成功添加队友: " + new_member_name_buffer_;
    new_member_name_buffer_.clear();

    RefreshTeamData();
}

void TeamScreen::HandleEquipMember(int memberIndex) {
    if (!game_ || memberIndex < 0 || memberIndex >= member_info_.size()) {
        status_message_ = "无效的成员索引";
        return;
    }
    
    status_message_ = "装备管理功能暂未实现，请在背包界面为成员装备物品";
}

void TeamScreen::HandleSwitchToMember(int memberIndex) {
    if (!game_ || memberIndex < 0 || memberIndex >= member_info_.size()) {
        status_message_ = "无效的成员索引";
        return;
    }
    
    if (!member_info_[memberIndex].isActive) {
        status_message_ = "只能切换到上场成员";
        return;
    }
    
    auto& player = game_->getPlayer();
    if (player.switchToMember(memberIndex)) {
        RefreshTeamData();
        status_message_ = "已切换到: " + member_info_[memberIndex].name;
    } else {
        status_message_ = "切换失败";
    }
}
