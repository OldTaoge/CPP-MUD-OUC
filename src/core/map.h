#pragma once
#include <string>
#include <vector>
#include <memory>
#include <random>
#include "../player/item.h"

// 可交互实体基类
class InteractableEntity {
public:
    std::string name;
    std::string description;
    bool isActive;
    
    InteractableEntity(const std::string& name, const std::string& description);
    virtual ~InteractableEntity() = default;
    
    virtual std::string getInteractionText() const = 0;
    virtual bool interact() = 0;
    virtual std::string getDisplayInfo() const = 0;
};

// NPC类
class NPC : public InteractableEntity {
public:
    std::string dialogue;
    std::vector<std::string> quests;
    bool hasQuest;
    
    NPC(const std::string& name, const std::string& description, 
        const std::string& dialogue, const std::vector<std::string>& quests = {});
    
    std::string getInteractionText() const override;
    bool interact() override;
    std::string getDisplayInfo() const override;
    
    std::string getRandomDialogue() const;
};

// 敌对实体类
class Enemy : public InteractableEntity {
public:
    int health;
    int attack;
    int defense;
    int expReward;
    std::vector<Item> lootTable;
    
    Enemy(const std::string& name, const std::string& description,
          int health, int attack, int defense, int expReward);
    
    std::string getInteractionText() const override;
    bool interact() override;
    std::string getDisplayInfo() const override;
    
    void addLoot(const Item& item);
    std::vector<Item> getLoot() const;
    bool isAlive() const;
    void takeDamage(int damage);
};

// 可拾取物品类
class CollectibleItem : public InteractableEntity {
public:
    Item item;
    bool isCollected;
    
    CollectibleItem(const std::string& name, const std::string& description, const Item& item);
    
    std::string getInteractionText() const override;
    bool interact() override;
    std::string getDisplayInfo() const override;
    
    Item collect();
    bool canCollect() const;
};

// 地图区域类
class MapArea {
public:
    std::string name;
    std::string description;
    std::vector<std::unique_ptr<NPC>> npcs;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<CollectibleItem>> collectibles;
    
    MapArea(const std::string& name, const std::string& description);
    
    void addNPC(std::unique_ptr<NPC> npc);
    void addEnemy(std::unique_ptr<Enemy> enemy);
    void addCollectible(std::unique_ptr<CollectibleItem> collectible);
    
    std::string getAreaInfo() const;
    std::vector<std::string> getInteractableList() const;
    
    // 获取特定类型的可交互实体
    std::vector<NPC*> getNPCs() const;
    std::vector<Enemy*> getEnemies() const;
    std::vector<CollectibleItem*> getCollectibles() const;
};

// 地图管理类
class MapManager {
public:
    static const int MAP_SIZE = 5; // 5个区块
    
    MapManager();
    ~MapManager();
    
    void initializeMap();
    MapArea* getArea(int index);
    std::string getMapOverview() const;
    
    // 移动相关
    bool canMoveToArea(int areaIndex) const;
    void moveToArea(int areaIndex);
    int getCurrentArea() const;
    
    // 战斗相关
    std::vector<Enemy*> getEnemiesInCurrentArea() const;
    void removeEnemyFromCurrentArea(Enemy* enemy);
    
    // 收集物品相关
    std::vector<CollectibleItem*> getCollectiblesInCurrentArea() const;
    void removeCollectibleFromCurrentArea(CollectibleItem* collectible);
    
    // 获取当前区域信息
    std::string getCurrentAreaInfo() const;
    
private:
    std::vector<std::unique_ptr<MapArea>> areas_;
    int currentArea_;
    
    void createForestArea();      // 森林区域
    void createVillageArea();    // 村庄区域
    void createDungeonArea();    // 地下城区域
    void createMountainArea();   // 山地区域
    void createCastleArea();     // 城堡区域
};
