#pragma once
#include <string>
#include <vector>
#include <memory>

// 物品类型枚举
enum class ItemType {
    WEAPON,
    ARTIFACT,
    FOOD,
    MATERIAL
};

// 武器类型枚举
enum class WeaponType {
    ONE_HANDED_SWORD,
    TWO_HANDED_SWORD,
    BOW,
    CATALYST
};

// 圣遗物类型枚举
enum class ArtifactType {
    FLOWER_OF_LIFE,
    PLUME_OF_DEATH,
    SANDS_OF_EON,
    GOBLET_OF_EONOTHEM,
    CIRCLET_OF_LOGOS
};

// 食物类型枚举
enum class FoodType {
    RECOVERY,
    ATTACK,
    ADVENTURE,
    DEFENSE
};

// 材料类型枚举
enum class MaterialType {
    MONSTER_DROP,
    COOKING_INGREDIENT
};

// 物品稀有度枚举
enum class Rarity {
    ONE_STAR = 1,
    TWO_STAR = 2,
    THREE_STAR = 3,
    FOUR_STAR = 4,
    FIVE_STAR = 5
};

// 基础物品类
class Item {
public:
    Item(const std::string& name, ItemType type, Rarity rarity, const std::string& description);
    virtual ~Item() = default;

    // Getters
    std::string getName() const { return name_; }
    ItemType getType() const { return type_; }
    Rarity getRarity() const { return rarity_; }
    std::string getDescription() const { return description_; }
    int getQuantity() const { return quantity_; }
    void setQuantity(int quantity) { quantity_ = quantity; }

    // 获取稀有度字符串表示
    std::string getRarityString() const;

    // 获取类型字符串表示
    virtual std::string getTypeString() const = 0;

    // 获取详细信息
    virtual std::string getDetailedInfo() const = 0;

protected:
    std::string name_;
    ItemType type_;
    Rarity rarity_;
    std::string description_;
    int quantity_;
};

// 武器类
class Weapon : public Item {
public:
    Weapon(const std::string& name, WeaponType weaponType, Rarity rarity,
           const std::string& description, int attackPower, int durability);

    WeaponType getWeaponType() const { return weaponType_; }
    int getAttackPower() const { return attackPower_; }
    int getDurability() const { return durability_; }
    void setDurability(int durability) { durability_ = durability; }

    std::string getTypeString() const override;
    std::string getDetailedInfo() const override;

private:
    WeaponType weaponType_;
    int attackPower_;
    int durability_;
};

// 圣遗物类
class Artifact : public Item {
public:
    Artifact(const std::string& name, ArtifactType artifactType, Rarity rarity,
             const std::string& description, const std::string& mainStat,
             const std::vector<std::string>& subStats);

    ArtifactType getArtifactType() const { return artifactType_; }
    std::string getMainStat() const { return mainStat_; }
    std::vector<std::string> getSubStats() const { return subStats_; }

    std::string getTypeString() const override;
    std::string getDetailedInfo() const override;

private:
    ArtifactType artifactType_;
    std::string mainStat_;
    std::vector<std::string> subStats_;
};

// 食物类
class Food : public Item {
public:
    Food(const std::string& name, FoodType foodType, Rarity rarity,
         const std::string& description, int effectValue, int duration);

    FoodType getFoodType() const { return foodType_; }
    int getEffectValue() const { return effectValue_; }
    int getDuration() const { return duration_; }

    std::string getTypeString() const override;
    std::string getDetailedInfo() const override;

private:
    FoodType foodType_;
    int effectValue_;
    int duration_;
};

// 材料类
class Material : public Item {
public:
    Material(const std::string& name, MaterialType materialType, Rarity rarity,
             const std::string& description, bool isStackable = true);

    MaterialType getMaterialType() const { return materialType_; }
    bool isStackable() const { return isStackable_; }

    std::string getTypeString() const override;
    std::string getDetailedInfo() const override;

private:
    MaterialType materialType_;
    bool isStackable_;
};

// 物品工厂类
class ItemFactory {
public:
    static std::shared_ptr<Item> createWeapon(const std::string& name, WeaponType type, Rarity rarity);
    static std::shared_ptr<Item> createArtifact(const std::string& name, ArtifactType type, Rarity rarity);
    static std::shared_ptr<Item> createFood(const std::string& name, FoodType type, Rarity rarity);
    static std::shared_ptr<Item> createMaterial(const std::string& name, MaterialType type, Rarity rarity);
};
