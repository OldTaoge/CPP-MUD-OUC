#include "Player.h"
#include "item.h"
#include <iostream>

Player::Player(std::string name, int startX, int startY, int startHealth)
    : name(name), x(startX), y(startY), health(startHealth) {
    // 初始化物品栏为空
    // 可以在这里添加一些初始物品
    
    // 示例：给玩家一些初始物品
    Item healthPotion("生命药水", "恢复20点生命值", ItemType::CONSUMABLE,
                     50, 3, true, 0, 0, 0, 20);
    addItem(healthPotion);
    
    Item ironSword("铁剑", "一把普通的铁剑，提供5点攻击力", ItemType::WEAPON,
                  100, 1, false, 5, 0, 0, 0);
    addItem(ironSword);
    
    Item goldCoin("金币", "可以用来购买物品", ItemType::MATERIAL,
                 1, 10, true, 0, 0, 0, 0);
    addItem(goldCoin);
    
    // 添加一些初始任务
    // 任务1：蒙德城的委托
    Quest mondstadtQuest("mondstadt_1", "蒙德城的委托", "帮助蒙德城的居民解决问题。");
    mondstadtQuest.addObjective(QuestObjective("与城市守卫对话"));
    mondstadtQuest.addObjective(QuestObjective("收集5个苹果"));
    mondstadtQuest.reward = "铁剑、生命药水";
    mondstadtQuest.exp_reward = 50;
    mondstadtQuest.gold_reward = 100;
    addQuest(mondstadtQuest);
    
    // 任务2：冒险的开始
    Quest adventureQuest("adventure_1", "冒险的开始", "探索周围的区域，了解提瓦特大陆。");
    adventureQuest.addObjective(QuestObjective("离开蒙德城"));
    adventureQuest.addObjective(QuestObjective("探索低语森林"));
    adventureQuest.reward = "冒险笔记、地图";
    adventureQuest.exp_reward = 30;
    adventureQuest.gold_reward = 50;
    addQuest(adventureQuest);
    
    // 任务3：元素能量
    Quest elementQuest("element_1", "元素能量", "了解提瓦特大陆的元素之力。");
    elementQuest.addObjective(QuestObjective("与丽莎对话"));
    elementQuest.addObjective(QuestObjective("收集3个元素碎片"));
    elementQuest.reward = "元素指南、魔法书";
    elementQuest.exp_reward = 40;
    elementQuest.gold_reward = 80;
    addQuest(elementQuest);
}

// 添加物品到物品栏
void Player::addItem(const Item& item) {
    // 如果物品可堆叠，检查是否已存在同名物品
    if (item.isStackable) {
        for (auto& existingItem : inventory) {
            if (existingItem.name == item.name) {
                existingItem.addStack(item.stackSize);
                return;
            }
        }
    }
    // 如果物品不可堆叠或者物品栏中没有同名物品，则直接添加
    inventory.push_back(item);
}

// 从物品栏移除物品
bool Player::removeItem(int index) {
    if (index < 0 || index >= inventory.size()) {
        return false;
    }
    inventory.erase(inventory.begin() + index);
    return true;
}

// 查找物品
Item* Player::findItem(const std::string& itemName) {
    for (auto& item : inventory) {
        if (item.name == itemName) {
            return &item;
        }
    }
    return nullptr;
}

// 使用物品
bool Player::useItem(int index) {
    if (index < 0 || index >= inventory.size()) {
        return false;
    }
    
    Item& item = inventory[index];
    if (item.use()) {
        // 如果物品堆叠数量变为0，则从物品栏中移除
        if (item.stackSize <= 0) {
            removeItem(index);
        }
        return true;
    }
    return false;
}

// 添加任务
void Player::addQuest(const Quest& quest) {
    quests_.push_back(quest);
}

// 获取所有任务
const std::vector<Quest>& Player::getQuests() const {
    return quests_;
}

// 查找任务
Quest* Player::findQuest(const std::string& questId) {
    for (auto& quest : quests_) {
        if (quest.id == questId) {
            return &quest;
        }
    }
    return nullptr;
}

// 开始任务
bool Player::startQuest(const std::string& questId) {
    Quest* quest = findQuest(questId);
    if (!quest) {
        return false;
    }
    
    quest->start();
    return true;
}

// 完成任务
bool Player::completeQuest(const std::string& questId) {
    Quest* quest = findQuest(questId);
    if (!quest) {
        return false;
    }
    
    return quest->complete();
}

// 更新任务目标进度
bool Player::updateQuestObjective(const std::string& questId, int objectiveIndex, int amount) {
    Quest* quest = findQuest(questId);
    if (!quest) {
        return false;
    }
    
    return quest->updateObjective(objectiveIndex, amount);
}

// 获取任务奖励
bool Player::claimQuestReward(const std::string& questId) {
    Quest* quest = findQuest(questId);
    if (!quest || quest->status != QuestStatus::COMPLETED) {
        return false;
    }
    
    // 发放金币奖励
    if (quest->gold_reward > 0) {
        // 查找金币物品
        Item* goldItem = findItem("金币");
        if (goldItem && goldItem->isStackable) {
            // 如果已有金币，增加堆叠数量
            goldItem->addStack(quest->gold_reward);
        } else {
            // 否则，添加新的金币物品
            Item newGold("金币", "可以用来购买物品", ItemType::MATERIAL,
                        1, quest->gold_reward, true, 0, 0, 0, 0);
            addItem(newGold);
        }
    }
    
    // 这里可以添加其他奖励的发放逻辑，如物品、经验等
    // 注意：经验值系统目前未实现，可以根据实际需求添加
    
    // 任务奖励已领取
    return true;
}