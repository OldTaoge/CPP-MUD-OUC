// =============================================
// 文件: team_member.h
// 描述: 队伍成员声明。包含属性、装备、战斗与状态接口。
// =============================================
#pragma once
#include "item.h"
#include <string>
#include <memory>

// 队伍成员状态枚举
enum class MemberStatus {
    ACTIVE,      // 上场
    STANDBY,     // 待命
    INJURED      // 受伤
};

class TeamMember {
public:
    TeamMember(const std::string& name, int level = 1);
    ~TeamMember() = default;

    // 基本属性
    std::string getName() const { return name_; }
    int getLevel() const { return level_; }
    void setLevel(int level) { level_ = level; }
    
    // 队伍状态管理
    MemberStatus getStatus() const { return status_; }
    void setStatus(MemberStatus status) { status_ = status; }
    bool isActive() const { return status_ == MemberStatus::ACTIVE; }
    bool canBeActive() const { return status_ != MemberStatus::INJURED || getCurrentHealth() > 0; }

    // 基础属性
    int getBaseHealth() const { return baseHealth_; }
    int getBaseAttack() const { return baseAttack_; }
    int getBaseDefense() const { return baseDefense_; }

    // 装备系统
    bool equipWeapon(std::shared_ptr<Weapon> weapon);
    bool equipArtifact(std::shared_ptr<Artifact> artifact);
    void unequipWeapon();
    void unequipArtifact();

    std::shared_ptr<Weapon> getEquippedWeapon() const { return equippedWeapon_; }
    std::shared_ptr<Artifact> getEquippedArtifact() const { return equippedArtifact_; }

    // 计算总属性（包含装备加成）
    int getTotalHealth() const;
    int getTotalAttack() const;
    int getTotalDefense() const;

    // 战斗相关
    int getCurrentHealth() const { return currentHealth_; }
    void setCurrentHealth(int health);
    void takeDamage(int damage);
    void heal(int amount);
    bool isAlive() const { return currentHealth_ > 0; }
    void resetHealth() { currentHealth_ = getTotalHealth(); }

private:
    std::string name_;
    int level_;
    int currentHealth_;
    MemberStatus status_;  // 队伍状态

    // 基础属性
    int baseHealth_;
    int baseAttack_;
    int baseDefense_;

    // 装备
    std::shared_ptr<Weapon> equippedWeapon_;
    std::shared_ptr<Artifact> equippedArtifact_;
};
