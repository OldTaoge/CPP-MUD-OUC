// =============================================
// 文件: item.cpp
// 描述: 物品系统实现。通用信息、具体子类与物品工厂。
// =============================================
#include "item.h"
#include <sstream>

// 基础物品类实现
Item::Item(const std::string& name, ItemType type, Rarity rarity, const std::string& description)
    : name_(name), type_(type), rarity_(rarity), description_(description), quantity_(1) {
}

std::string Item::getRarityString() const {
    switch (rarity_) {
        case Rarity::ONE_STAR: return "1星";
        case Rarity::TWO_STAR: return "2星";
        case Rarity::THREE_STAR: return "3星";
        case Rarity::FOUR_STAR: return "4星";
        case Rarity::FIVE_STAR: return "5星";
        default: return "?";
    }
}

// 武器类实现
Weapon::Weapon(const std::string& name, WeaponType weaponType, Rarity rarity,
               const std::string& description, int attackPower, int durability)
    : Item(name, ItemType::WEAPON, rarity, description),
      weaponType_(weaponType), attackPower_(attackPower), durability_(durability) {
}

std::string Weapon::getTypeString() const {
    switch (weaponType_) {
        case WeaponType::ONE_HANDED_SWORD: return "单手剑";
        case WeaponType::TWO_HANDED_SWORD: return "双手剑";
        case WeaponType::BOW: return "弓箭";
        case WeaponType::CATALYST: return "法器";
        default: return "未知武器";
    }
}

std::string Weapon::getDetailedInfo() const {
    std::stringstream ss;
    ss << "武器类型: " << getTypeString() << "\n";
    ss << "稀有度: " << getRarityString() << "\n";
    ss << "攻击力: " << attackPower_ << "\n";
    ss << "耐久度: " << durability_ << "\n";
    ss << "描述: " << getDescription();
    return ss.str();
}

// 圣遗物类实现
Artifact::Artifact(const std::string& name, ArtifactType artifactType, Rarity rarity,
                   const std::string& description, const std::string& mainStat,
                   const std::vector<std::string>& subStats)
    : Item(name, ItemType::ARTIFACT, rarity, description),
      artifactType_(artifactType), mainStat_(mainStat), subStats_(subStats) {
}

std::string Artifact::getTypeString() const {
    switch (artifactType_) {
        case ArtifactType::FLOWER_OF_LIFE: return "生之花";
        case ArtifactType::PLUME_OF_DEATH: return "死之羽";
        case ArtifactType::SANDS_OF_EON: return "时之沙";
        case ArtifactType::GOBLET_OF_EONOTHEM: return "空之杯";
        case ArtifactType::CIRCLET_OF_LOGOS: return "理之冠";
        default: return "未知圣遗物";
    }
}

std::string Artifact::getDetailedInfo() const {
    std::stringstream ss;
    ss << "圣遗物类型: " << getTypeString() << "\n";
    ss << "稀有度: " << getRarityString() << "\n";
    ss << "主属性: " << mainStat_ << "\n";
    if (!subStats_.empty()) {
        ss << "副属性:\n";
        for (const auto& stat : subStats_) {
            ss << "  - " << stat << "\n";
        }
    }
    ss << "描述: " << getDescription();
    return ss.str();
}

// 食物类实现
Food::Food(const std::string& name, FoodType foodType, Rarity rarity,
           const std::string& description, int effectValue, int duration)
    : Item(name, ItemType::FOOD, rarity, description),
      foodType_(foodType), effectValue_(effectValue), duration_(duration) {
}

std::string Food::getTypeString() const {
    switch (foodType_) {
        case FoodType::RECOVERY: return "恢复类";
        case FoodType::ATTACK: return "攻击类";
        case FoodType::ADVENTURE: return "冒险类";
        case FoodType::DEFENSE: return "防御类";
        default: return "未知食物";
    }
}

std::string Food::getDetailedInfo() const {
    std::stringstream ss;
    ss << "食物类型: " << getTypeString() << "\n";
    ss << "稀有度: " << getRarityString() << "\n";
    ss << "效果值: " << effectValue_ << "\n";
    ss << "持续时间: " << duration_ << " 回合\n";
    ss << "描述: " << getDescription();
    return ss.str();
}

// 材料类实现
Material::Material(const std::string& name, MaterialType materialType, Rarity rarity,
                   const std::string& description, bool isStackable)
    : Item(name, ItemType::MATERIAL, rarity, description),
      materialType_(materialType), isStackable_(isStackable) {
}

std::string Material::getTypeString() const {
    switch (materialType_) {
        case MaterialType::MONSTER_DROP: return "怪物掉落物";
        case MaterialType::COOKING_INGREDIENT: return "烹饪原料";
        default: return "未知材料";
    }
}

std::string Material::getDetailedInfo() const {
    std::stringstream ss;
    ss << "材料类型: " << getTypeString() << "\n";
    ss << "稀有度: " << getRarityString() << "\n";
    ss << "可堆叠: " << (isStackable_ ? "是" : "否") << "\n";
    ss << "数量: " << getQuantity() << "\n";
    ss << "描述: " << getDescription();
    return ss.str();
}

// 物品工厂实现
std::shared_ptr<Item> ItemFactory::createWeapon(const std::string& name, WeaponType type, Rarity rarity) {
    std::string description;
    int attackPower = 0;
    int durability = 100;

    // 根据稀有度和类型设置属性
    switch (rarity) {
        case Rarity::ONE_STAR:
            attackPower = 40 + static_cast<int>(type) * 10;
            description = "基础品质的" + name;
            break;
        case Rarity::TWO_STAR:
            attackPower = 60 + static_cast<int>(type) * 15;
            description = "普通品质的" + name;
            break;
        case Rarity::THREE_STAR:
            attackPower = 80 + static_cast<int>(type) * 20;
            description = "优秀品质的" + name;
            break;
        case Rarity::FOUR_STAR:
            attackPower = 100 + static_cast<int>(type) * 25;
            description = "精良品质的" + name;
            break;
        case Rarity::FIVE_STAR:
            attackPower = 120 + static_cast<int>(type) * 30;
            description = "传说品质的" + name;
            break;
    }

    return std::make_shared<Weapon>(name, type, rarity, description, attackPower, durability);
}

std::shared_ptr<Item> ItemFactory::createArtifact(const std::string& name, ArtifactType type, Rarity rarity) {
    std::string description;
    std::string mainStat;
    std::vector<std::string> subStats;

    // 根据类型设置主属性
    switch (type) {
        case ArtifactType::FLOWER_OF_LIFE:
            mainStat = "生命值 +" + std::to_string(1000 + static_cast<int>(rarity) * 200);
            break;
        case ArtifactType::PLUME_OF_DEATH:
            mainStat = "攻击力 +" + std::to_string(50 + static_cast<int>(rarity) * 10);
            break;
        case ArtifactType::SANDS_OF_EON:
            mainStat = "暂无效果";
            break;
        case ArtifactType::GOBLET_OF_EONOTHEM:
            mainStat = "暂无效果";
            break;
        case ArtifactType::CIRCLET_OF_LOGOS:
            mainStat = "暂无效果";
            break;
    }

    // 根据稀有度设置描述和副属性
    switch (rarity) {
        case Rarity::ONE_STAR:
            description = "基础品质的圣遗物";
            break;
        case Rarity::TWO_STAR:
            description = "普通品质的圣遗物";
            subStats = {"防御力 +10"};
            break;
        case Rarity::THREE_STAR:
            description = "优秀品质的圣遗物";
            subStats = {"防御力 +15", "元素精通 +5"};
            break;
        case Rarity::FOUR_STAR:
            description = "精良品质的圣遗物";
            subStats = {"防御力 +20", "元素精通 +10", "暴击伤害 +5%"};
            break;
        case Rarity::FIVE_STAR:
            description = "传说品质的圣遗物";
            subStats = {"防御力 +25", "元素精通 +15", "暴击伤害 +10%", "治疗加成 +5%"};
            break;
    }

    return std::make_shared<Artifact>(name, type, rarity, description, mainStat, subStats);
}

std::shared_ptr<Item> ItemFactory::createFood(const std::string& name, FoodType type, Rarity rarity) {
    std::string description;
    int effectValue = 0;
    int duration = 0;

    // 根据食物类型和稀有度设置属性
    switch (rarity) {
        case Rarity::ONE_STAR:
            duration = 2;
            switch (type) {
                case FoodType::RECOVERY: effectValue = 100; break;
                case FoodType::ATTACK: effectValue = 20; break;
                case FoodType::ADVENTURE: effectValue = 15; break;
                case FoodType::DEFENSE: effectValue = 25; break;
            }
            description = "基础品质的" + name;
            break;
        case Rarity::TWO_STAR:
            duration = 3;
            switch (type) {
                case FoodType::RECOVERY: effectValue = 150; break;
                case FoodType::ATTACK: effectValue = 30; break;
                case FoodType::ADVENTURE: effectValue = 20; break;
                case FoodType::DEFENSE: effectValue = 35; break;
            }
            description = "普通品质的" + name;
            break;
        case Rarity::THREE_STAR:
            duration = 4;
            switch (type) {
                case FoodType::RECOVERY: effectValue = 200; break;
                case FoodType::ATTACK: effectValue = 40; break;
                case FoodType::ADVENTURE: effectValue = 25; break;
                case FoodType::DEFENSE: effectValue = 45; break;
            }
            description = "优秀品质的" + name;
            break;
        case Rarity::FOUR_STAR:
            duration = 5;
            switch (type) {
                case FoodType::RECOVERY: effectValue = 250; break;
                case FoodType::ATTACK: effectValue = 50; break;
                case FoodType::ADVENTURE: effectValue = 30; break;
                case FoodType::DEFENSE: effectValue = 55; break;
            }
            description = "精良品质的" + name;
            break;
        case Rarity::FIVE_STAR:
            duration = 6;
            switch (type) {
                case FoodType::RECOVERY: effectValue = 300; break;
                case FoodType::ATTACK: effectValue = 60; break;
                case FoodType::ADVENTURE: effectValue = 35; break;
                case FoodType::DEFENSE: effectValue = 65; break;
            }
            description = "传说品质的" + name;
            break;
    }

    return std::make_shared<Food>(name, type, rarity, description, effectValue, duration);
}

std::shared_ptr<Item> ItemFactory::createMaterial(const std::string& name, MaterialType type, Rarity rarity) {
    std::string description;

    switch (rarity) {
        case Rarity::ONE_STAR:
            description = "普通的" + name;
            break;
        case Rarity::TWO_STAR:
            description = "优质的" + name;
            break;
        case Rarity::THREE_STAR:
            description = "精致的" + name;
            break;
        case Rarity::FOUR_STAR:
            description = "珍贵的" + name;
            break;
        case Rarity::FIVE_STAR:
            description = "传奇的" + name;
            break;
    }

    bool isStackable = (type == MaterialType::MONSTER_DROP || type == MaterialType::COOKING_INGREDIENT);
    return std::make_shared<Material>(name, type, rarity, description, isStackable);
}
