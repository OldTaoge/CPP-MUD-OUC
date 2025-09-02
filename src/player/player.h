#pragma once
#include <string>
#include <vector>
#include "item.h"
#include "quest.h"

class Player {
public:
    std::string name;
    int x, y; // 玩家坐标
    int health;
    std::vector<Item> inventory; // 玩家物品栏

    Player(std::string name, int startX, int startY, int startHealth);
    
    // 添加物品到物品栏
    void addItem(const Item& item);
    
    // 从物品栏移除物品
    bool removeItem(int index);
    
    // 查找物品
    Item* findItem(const std::string& itemName);
    
    // 使用物品
    bool useItem(int index);
    
    // 任务相关方法
    // 添加任务
    void addQuest(const Quest& quest);
    
    // 获取所有任务
    const std::vector<Quest>& getQuests() const;
    
    // 查找任务
    Quest* findQuest(const std::string& questId);
    
    // 开始任务
    bool startQuest(const std::string& questId);
    
    // 完成任务
    bool completeQuest(const std::string& questId);
    
    // 更新任务目标进度
    bool updateQuestObjective(const std::string& questId, int objectiveIndex, int amount = 1);
    
    // 获取任务奖励
    bool claimQuestReward(const std::string& questId);
    
private:
    std::vector<Quest> quests_;  // 玩家的任务列表
};