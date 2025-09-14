#include "item.h"
#include <iostream>

// 构造函数实现
Item::Item(std::string name,
           std::string description,
           ItemType type,
           int value,
           int stackSize,
           bool isStackable,
           int attackBonus,
           int defenseBonus,
           int healthBonus,
           int healthRestore)
    : name(name),
      description(description),
      type(type),
      value(value),
      stackSize(stackSize),
      isStackable(isStackable),
      attackBonus(attackBonus),
      defenseBonus(defenseBonus),
      healthBonus(healthBonus),
      healthRestore(healthRestore) {
}

// 获取物品类型的字符串表示
std::string Item::getTypeString() const {
    switch (type) {
        case ItemType::WEAPON:
            return "武器";
        case ItemType::ARMOR:
            return "防具";
        case ItemType::CONSUMABLE:
            return "消耗品";
        case ItemType::QUEST:
            return "任务物品";
        case ItemType::MATERIAL:
            return "材料";
        case ItemType::KEY:
            return "钥匙";
        default:
            return "未知";
    }
}

// 物品使用效果
bool Item::use() {
    if (type == ItemType::CONSUMABLE && healthRestore > 0) {
        // 消耗品使用逻辑
        reduceStack(1);
        return true;
    }
    // 其他类型物品的使用逻辑可以在这里添加
    return false;
}

// 增加物品堆叠数量
bool Item::addStack(int amount) {
    if (!isStackable) {
        return false;
    }
    stackSize += amount;
    return true;
}

// 减少物品堆叠数量
bool Item::reduceStack(int amount) {
    if (stackSize < amount) {
        return false;
    }
    stackSize -= amount;
    return true;
}