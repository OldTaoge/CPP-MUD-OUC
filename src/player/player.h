// =============================================
// 文件: player.h
// 描述: 玩家实体，包含名称、坐标、等级、经验、队伍与背包。
// 说明: 仅声明接口；实现见 player.cpp。
// =============================================
#pragma once
#include <string>
#include <vector>
#include "../core/inventory.h"
#include "../core/team_member.h"

// 玩家数据与操作
class Player {
public:
    std::string name;
    int x, y; // 玩家坐标
    int level;
    int experience;

    // 队伍成员 ---------------------------------------------------------------
    std::vector<std::shared_ptr<TeamMember>> teamMembers;
    std::shared_ptr<TeamMember> activeMember; // 当前活跃的队伍成员
    static const int MAX_ACTIVE_MEMBERS = 4; // 最大上场人数

    // 背包 -------------------------------------------------------------------
    Inventory inventory;

    Player(std::string name, int startX, int startY);

    // 队伍成员管理 -----------------------------------------------------------
    void addTeamMember(const std::string& name, int level = 1);
    void setActiveMember(int index);
    std::shared_ptr<TeamMember> getActiveMember() const { return activeMember; }
    std::vector<std::shared_ptr<TeamMember>> getTeamMembers() const { return teamMembers; }
    
    // 队伍配置管理 -----------------------------------------------------------
    bool setMemberActive(int index, bool active);
    std::vector<std::shared_ptr<TeamMember>> getActiveMembers() const;
    std::vector<std::shared_ptr<TeamMember>> getStandbyMembers() const;
    int getActiveCount() const;
    bool canAddActiveMembers() const { return getActiveCount() < MAX_ACTIVE_MEMBERS; }
    
    // 队友切换 ---------------------------------------------------------------
    bool switchToNextActiveMember();
    bool switchToPreviousActiveMember();
    bool switchToMember(int index);

    // 物品管理 ---------------------------------------------------------------
    InventoryResult addItemToInventory(std::shared_ptr<Item> item);
    InventoryResult removeItemFromInventory(const std::string& itemName, int quantity = 1);
    InventoryResult useItem(const std::string& itemName);

    // 装备管理（为指定队伍成员装备）-----------------------------------------
    bool equipWeaponForMember(int memberIndex, const std::string& weaponName);
    bool equipArtifactForMember(int memberIndex, const std::string& artifactName);
    void unequipWeaponFromMember(int memberIndex);
    void unequipArtifactFromMember(int memberIndex);

    // 战斗相关 ---------------------------------------------------------------
    int getTotalAttackPower() const;
    int getTotalDefensePower() const;

    // 状态管理 ---------------------------------------------------------------
    void takeDamage(int damage);
    void heal(int amount);
    bool isAlive() const;
};