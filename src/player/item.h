#pragma once
#include <string>
#include <vector>

// 物品类型枚举
enum class ItemType {
    WEAPON,      // 武器
    ARMOR,       // 防具
    CONSUMABLE,  // 消耗品
    QUEST,       // 任务物品
    MATERIAL,    // 材料
    KEY          // 钥匙
};

// 物品类
class Item {
public:
    std::string name;            // 物品名称
    std::string description;     // 物品描述
    ItemType type;               // 物品类型
    int value;                   // 物品价值
    int stackSize;               // 堆叠数量
    bool isStackable;            // 是否可堆叠
    
    // 物品效果属性（根据物品类型不同可能有不同用途）
    int attackBonus;             // 攻击加成
    int defenseBonus;            // 防御加成
    int healthBonus;             // 生命加成
    int healthRestore;           // 生命恢复
    
    // 构造函数
    Item(std::string name = "未知物品",
         std::string description = "未知的物品",
         ItemType type = ItemType::MATERIAL,
         int value = 0,
         int stackSize = 1,
         bool isStackable = false,
         int attackBonus = 0,
         int defenseBonus = 0,
         int healthBonus = 0,
         int healthRestore = 0);
    
    // 获取物品类型的字符串表示
    std::string getTypeString() const;
    
    // 物品使用效果
    bool use();
    
    // 增加物品堆叠数量
    bool addStack(int amount = 1);
    
    // 减少物品堆叠数量
    bool reduceStack(int amount = 1);
};