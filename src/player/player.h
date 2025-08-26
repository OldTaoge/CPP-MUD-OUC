#pragma once
#include <string>

class Player {
public:
    std::string name;
    int x, y; // 玩家坐标
    int health;
    // ... 其他玩家属性，如攻击力、防御力、物品栏等

    Player(std::string name, int startX, int startY, int startHealth);
};