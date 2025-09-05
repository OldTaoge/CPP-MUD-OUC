#pragma once
#include "item.h"
#include <string>
#include <memory>

class TeamMember {
public:
    TeamMember(const std::string& name, int level = 1);
    ~TeamMember() = default;

    // 基本属性
    std::string getName() const { return name_; }
    int getLevel() const { return level_; }
    void setLevel(int level) { level_ = level; }

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

    // 基础属性
    int baseHealth_;
    int baseAttack_;
    int baseDefense_;

    // 装备
    std::shared_ptr<Weapon> equippedWeapon_;
    std::shared_ptr<Artifact> equippedArtifact_;
};
