#pragma once
#include <string>
#include <vector>
#include "../core/inventory.h"
#include "../core/team_member.h"

class Player {
public:
    std::string name;
    int x, y; // 玩家坐标
    int level;
    int experience;

    // 队伍成员
    std::vector<std::shared_ptr<TeamMember>> teamMembers;
    std::shared_ptr<TeamMember> activeMember; // 当前活跃的队伍成员

    // 背包
    Inventory inventory;

    Player(std::string name, int startX, int startY);

    // 队伍成员管理
    void addTeamMember(const std::string& name, int level = 1);
    void setActiveMember(int index);
    std::shared_ptr<TeamMember> getActiveMember() const { return activeMember; }
    std::vector<std::shared_ptr<TeamMember>> getTeamMembers() const { return teamMembers; }

    // 物品管理
    InventoryResult addItemToInventory(std::shared_ptr<Item> item);
    InventoryResult removeItemFromInventory(const std::string& itemName, int quantity = 1);
    InventoryResult useItem(const std::string& itemName);

    // 装备管理（为指定队伍成员装备）
    bool equipWeaponForMember(int memberIndex, const std::string& weaponName);
    bool equipArtifactForMember(int memberIndex, const std::string& artifactName);
    void unequipWeaponFromMember(int memberIndex);
    void unequipArtifactFromMember(int memberIndex);

    // 战斗相关
    int getTotalAttackPower() const;
    int getTotalDefensePower() const;

    // 状态管理
    void takeDamage(int damage);
    void heal(int amount);
    bool isAlive() const;
};