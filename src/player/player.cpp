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