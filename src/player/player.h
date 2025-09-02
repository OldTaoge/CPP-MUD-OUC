#pragma once
#include <string>
#include <vector>
#include "item.h"

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
};