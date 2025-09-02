#include "map.h"
#include <algorithm>
#include <sstream>
#include <iostream>

// InteractableEntity 实现
InteractableEntity::InteractableEntity(const std::string& name, const std::string& description)
    : name(name), description(description), isActive(true) {}

// NPC 实现
NPC::NPC(const std::string& name, const std::string& description, 
         const std::string& dialogue, const std::vector<std::string>& quests)
    : InteractableEntity(name, description), dialogue(dialogue), quests(quests), hasQuest(!quests.empty()) {}

std::string NPC::getInteractionText() const {
    return "与 " + name + " 对话";
}

bool NPC::interact() {
    return true; // NPC交互总是成功
}

std::string NPC::getDisplayInfo() const {
    std::string info = "NPC: " + name + "\n";
    info += "描述: " + description + "\n";
    info += "对话: " + dialogue + "\n";
    if (hasQuest) {
        info += "状态: 有任务可接\n";
    } else {
        info += "状态: 无任务\n";
    }
    return info;
}

std::string NPC::getRandomDialogue() const {
    return dialogue; // 简化版本，返回固定对话
}

// Enemy 实现
Enemy::Enemy(const std::string& name, const std::string& description,
             int health, int attack, int defense, int expReward)
    : InteractableEntity(name, description), health(health), attack(attack), 
      defense(defense), expReward(expReward) {}

std::string Enemy::getInteractionText() const {
    return "攻击 " + name;
}

bool Enemy::interact() {
    return isAlive(); // 只有活着的敌人才能被攻击
}

std::string Enemy::getDisplayInfo() const {
    std::string info = "敌人: " + name + "\n";
    info += "描述: " + description + "\n";
    info += "生命值: " + std::to_string(health) + "\n";
    info += "攻击力: " + std::to_string(attack) + "\n";
    info += "防御力: " + std::to_string(defense) + "\n";
    info += "经验奖励: " + std::to_string(expReward) + "\n";
    info += "状态: " + std::string(isAlive() ? "存活" : "已死亡") + "\n";
    return info;
}

void Enemy::addLoot(const Item& item) {
    lootTable.push_back(item);
}

std::vector<Item> Enemy::getLoot() const {
    return lootTable;
}

bool Enemy::isAlive() const {
    return health > 0;
}

void Enemy::takeDamage(int damage) {
    int actualDamage = std::max(1, damage - defense);
    health = std::max(0, health - actualDamage);
}

// CollectibleItem 实现
CollectibleItem::CollectibleItem(const std::string& name, const std::string& description, const Item& item)
    : InteractableEntity(name, description), item(item), isCollected(false) {}

std::string CollectibleItem::getInteractionText() const {
    return "拾取 " + name;
}

bool CollectibleItem::interact() {
    if (canCollect()) {
        isCollected = true;
        return true;
    }
    return false;
}

std::string CollectibleItem::getDisplayInfo() const {
    std::string info = "可拾取物品: " + name + "\n";
    info += "描述: " + description + "\n";
    info += "物品: " + item.name + "\n";
    info += "状态: " + std::string(isCollected ? "已拾取" : "可拾取") + "\n";
    return info;
}

Item CollectibleItem::collect() {
    if (canCollect()) {
        isCollected = true;
        return item;
    }
    return Item(); // 返回空物品
}

bool CollectibleItem::canCollect() const {
    return !isCollected && isActive;
}

// MapArea 实现
MapArea::MapArea(const std::string& name, const std::string& description)
    : name(name), description(description) {}

void MapArea::addNPC(std::unique_ptr<NPC> npc) {
    npcs.push_back(std::move(npc));
}

void MapArea::addEnemy(std::unique_ptr<Enemy> enemy) {
    enemies.push_back(std::move(enemy));
}

void MapArea::addCollectible(std::unique_ptr<CollectibleItem> collectible) {
    collectibles.push_back(std::move(collectible));
}

std::string MapArea::getAreaInfo() const {
    std::string info = "区域: " + name + "\n";
    info += "描述: " + description + "\n";
    info += "NPC数量: " + std::to_string(npcs.size()) + "\n";
    info += "敌人数量: " + std::to_string(enemies.size()) + "\n";
    info += "可拾取物品数量: " + std::to_string(collectibles.size()) + "\n";
    return info;
}

std::vector<std::string> MapArea::getInteractableList() const {
    std::vector<std::string> list;
    
    for (const auto& npc : npcs) {
        if (npc->isActive) {
            list.push_back("NPC: " + npc->name);
        }
    }
    
    for (const auto& enemy : enemies) {
        if (enemy->isActive && enemy->isAlive()) {
            list.push_back("敌人: " + enemy->name);
        }
    }
    
    for (const auto& collectible : collectibles) {
        if (collectible->isActive && collectible->canCollect()) {
            list.push_back("物品: " + collectible->name);
        }
    }
    
    return list;
}

std::vector<NPC*> MapArea::getNPCs() const {
    std::vector<NPC*> npcPtrs;
    for (const auto& npc : npcs) {
        if (npc->isActive) {
            npcPtrs.push_back(npc.get());
        }
    }
    return npcPtrs;
}

std::vector<Enemy*> MapArea::getEnemies() const {
    std::vector<Enemy*> enemyPtrs;
    for (const auto& enemy : enemies) {
        if (enemy->isActive && enemy->isAlive()) {
            enemyPtrs.push_back(enemy.get());
        }
    }
    return enemyPtrs;
}

std::vector<CollectibleItem*> MapArea::getCollectibles() const {
    std::vector<CollectibleItem*> collectiblePtrs;
    for (const auto& collectible : collectibles) {
        if (collectible->isActive && collectible->canCollect()) {
            collectiblePtrs.push_back(collectible.get());
        }
    }
    return collectiblePtrs;
}

// MapManager 实现
MapManager::MapManager() : currentArea_(0) {
    initializeMap();
}

MapManager::~MapManager() = default;

void MapManager::initializeMap() {
    createForestArea();
    createVillageArea();
    createDungeonArea();
    createMountainArea();
    createCastleArea();
}

void MapManager::createForestArea() {
    auto forest = std::make_unique<MapArea>("神秘森林", "一片充满神秘气息的森林，树木参天，光线昏暗。");
    
    // 添加NPC
    forest->addNPC(std::make_unique<NPC>("森林守护者", "一位年迈的森林守护者", 
        "欢迎来到神秘森林，这里充满了危险和机遇。", std::vector<std::string>{"寻找失落的护符"}));
    
    // 添加敌人
    forest->addEnemy(std::make_unique<Enemy>("森林狼", "凶猛的森林狼", 30, 8, 3, 15));
    forest->addEnemy(std::make_unique<Enemy>("毒蛇", "剧毒的毒蛇", 25, 12, 2, 20));
    
    // 添加可拾取物品
    forest->addCollectible(std::make_unique<CollectibleItem>("草药", "一株珍贵的草药", 
        Item("草药", "治疗药草", ItemType::CONSUMABLE, 5, 1, false, 0, 0, 0, 20)));
    
    areas_.push_back(std::move(forest));
}

void MapManager::createVillageArea() {
    auto village = std::make_unique<MapArea>("和平村庄", "一个宁静的小村庄，村民们过着平静的生活。");
    
    // 添加NPC
    village->addNPC(std::make_unique<NPC>("村长", "村庄的领导者", 
        "欢迎来到我们的村庄，这里很安全。", std::vector<std::string>{"帮助村民"}));
    village->addNPC(std::make_unique<NPC>("商人", "村庄的商人", 
        "来看看我的商品吧，价格公道。", std::vector<std::string>{}));
    
    // 添加敌人（村庄相对安全）
    village->addEnemy(std::make_unique<Enemy>("小偷", "鬼鬼祟祟的小偷", 20, 5, 1, 10));
    
    // 添加可拾取物品
    village->addCollectible(std::make_unique<CollectibleItem>("金币", "一些金币", 
        Item("金币", "通用货币", ItemType::MATERIAL, 10, 1, true)));
    
    areas_.push_back(std::move(village));
}

void MapManager::createDungeonArea() {
    auto dungeon = std::make_unique<MapArea>("古老地下城", "一个充满危险的地下城，传说中藏有宝藏。");
    
    // 添加NPC
    dungeon->addNPC(std::make_unique<NPC>("被困的冒险者", "一位被困的冒险者", 
        "救救我！我被困在这里很久了。", std::vector<std::string>{"营救冒险者"}));
    
    // 添加敌人
    dungeon->addEnemy(std::make_unique<Enemy>("骷髅战士", "不死族的战士", 40, 15, 8, 25));
    dungeon->addEnemy(std::make_unique<Enemy>("黑暗法师", "掌握黑暗魔法的法师", 35, 20, 5, 30));
    dungeon->addEnemy(std::make_unique<Enemy>("巨型蜘蛛", "巨大的蜘蛛怪物", 50, 18, 10, 35));
    
    // 添加可拾取物品
    dungeon->addCollectible(std::make_unique<CollectibleItem>("古老钥匙", "一把神秘的钥匙", 
        Item("古老钥匙", "开启某个神秘之门的钥匙", ItemType::KEY, 0, 1, false)));
    dungeon->addCollectible(std::make_unique<CollectibleItem>("魔法卷轴", "记载着魔法的卷轴", 
        Item("魔法卷轴", "记载着强大的魔法", ItemType::QUEST, 100, 1, false)));
    
    areas_.push_back(std::move(dungeon));
}

void MapManager::createMountainArea() {
    auto mountain = std::make_unique<MapArea>("险峻山峰", "高耸入云的山峰，环境恶劣但资源丰富。");
    
    // 添加NPC
    mountain->addNPC(std::make_unique<NPC>("登山向导", "经验丰富的登山向导", 
        "这座山峰很危险，需要专业的装备。", std::vector<std::string>{"收集矿石"}));
    
    // 添加敌人
    mountain->addEnemy(std::make_unique<Enemy>("山鹰", "凶猛的山鹰", 30, 12, 4, 18));
    mountain->addEnemy(std::make_unique<Enemy>("雪怪", "传说中的雪怪", 60, 25, 15, 40));
    
    // 添加可拾取物品
    mountain->addCollectible(std::make_unique<CollectibleItem>("稀有矿石", "珍贵的矿石", 
        Item("稀有矿石", "用于制作高级装备的矿石", ItemType::MATERIAL, 50, 1, false)));
    
    areas_.push_back(std::move(mountain));
}

void MapManager::createCastleArea() {
    auto castle = std::make_unique<MapArea>("古老城堡", "一座神秘的古老城堡，充满了未知的危险。");
    
    // 添加NPC
    castle->addNPC(std::make_unique<NPC>("城堡守卫", "忠诚的城堡守卫", 
        "这里是禁地，没有许可不能进入。", std::vector<std::string>{"获得进入许可"}));
    
    // 添加敌人
    castle->addEnemy(std::make_unique<Enemy>("骑士", "装备精良的骑士", 45, 20, 12, 30));
    castle->addEnemy(std::make_unique<Enemy>("魔法守卫", "城堡的魔法守卫", 40, 25, 8, 35));
    castle->addEnemy(std::make_unique<Enemy>("城堡领主", "城堡的主人", 80, 35, 20, 100));
    
    // 添加可拾取物品
    castle->addCollectible(std::make_unique<CollectibleItem>("王冠", "象征权力的王冠", 
        Item("王冠", "象征至高权力的王冠", ItemType::QUEST, 1000, 1, false)));
    
    areas_.push_back(std::move(castle));
}

MapArea* MapManager::getArea(int index) {
    if (index >= 0 && index < areas_.size()) {
        return areas_[index].get();
    }
    return nullptr;
}

std::string MapManager::getMapOverview() const {
    std::string overview = "=== 世界地图概览 ===\n\n";
    
    for (size_t i = 0; i < areas_.size(); ++i) {
        overview += std::to_string(i + 1) + ". " + areas_[i]->name;
        if (i == currentArea_) {
            overview += " [当前位置]";
        }
        overview += "\n";
        overview += "   " + areas_[i]->description + "\n";
        overview += "   可交互内容: " + std::to_string(areas_[i]->getInteractableList().size()) + " 项\n\n";
    }
    
    return overview;
}

bool MapManager::canMoveToArea(int areaIndex) const {
    return areaIndex >= 0 && areaIndex < areas_.size();
}

void MapManager::moveToArea(int areaIndex) {
    if (canMoveToArea(areaIndex)) {
        currentArea_ = areaIndex;
    }
}

int MapManager::getCurrentArea() const {
    return currentArea_;
}

std::vector<Enemy*> MapManager::getEnemiesInCurrentArea() const {
    if (currentArea_ >= 0 && currentArea_ < areas_.size()) {
        return areas_[currentArea_]->getEnemies();
    }
    return {};
}

void MapManager::removeEnemyFromCurrentArea(Enemy* enemy) {
    if (currentArea_ >= 0 && currentArea_ < areas_.size()) {
        auto& enemies = areas_[currentArea_]->enemies;
        enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
            [enemy](const std::unique_ptr<Enemy>& e) { return e.get() == enemy; }), enemies.end());
    }
}

std::vector<CollectibleItem*> MapManager::getCollectiblesInCurrentArea() const {
    if (currentArea_ >= 0 && currentArea_ < areas_.size()) {
        return areas_[currentArea_]->getCollectibles();
    }
    return {};
}

void MapManager::removeCollectibleFromCurrentArea(CollectibleItem* collectible) {
    if (currentArea_ >= 0 && currentArea_ < areas_.size()) {
        auto& collectibles = areas_[currentArea_]->collectibles;
        collectibles.erase(std::remove_if(collectibles.begin(), collectibles.end(),
            [collectible](const std::unique_ptr<CollectibleItem>& c) { return c.get() == collectible; }), collectibles.end());
    }
}

std::string MapManager::getCurrentAreaInfo() const {
    if (currentArea_ >= 0 && currentArea_ < areas_.size()) {
        return areas_[currentArea_]->getAreaInfo();
    }
    return "未知区域";
}
