#include "map.h"
#include <iostream>
#include <algorithm>
#include <sstream>

// ==================== MapBlock 基类实现 ====================

MapBlock::MapBlock(const std::string& name, MapBlockType type, int x, int y, 
                   const std::string& description)
    : name_(name), type_(type), x_(x), y_(y), description_(description), 
      state_(BlockState::UNLOCKED) {
    // 不在构造函数中调用纯虚函数，改为在子类构造函数中调用
}

InteractionResult MapBlock::interact(Player& player, InteractionType interactionType) {
    auto it = interactionHandlers_.find(interactionType);
    if (it != interactionHandlers_.end()) {
        return it->second(player);
    }
    return InteractionResult(false, "无法进行此交互");
}

bool MapBlock::canInteract(InteractionType interactionType) const {
    return interactionHandlers_.find(interactionType) != interactionHandlers_.end();
}

std::vector<InteractionType> MapBlock::getAvailableInteractions() const {
    std::vector<InteractionType> interactions;
    for (const auto& pair : interactionHandlers_) {
        interactions.push_back(pair.first);
    }
    return interactions;
}

std::string MapBlock::getDisplaySymbol() const {
    switch (type_) {
        case MapBlockType::TUTORIAL: return "T";
        case MapBlockType::STATUE_OF_SEVEN: return "S";
        case MapBlockType::BATTLE: return "B";
        case MapBlockType::DIALOGUE: return "D";
        case MapBlockType::CITY: return "C";
        default: return "?";
    }
}

std::string MapBlock::getDisplayName() const {
    return name_;
}

// ==================== TutorialBlock 实现 ====================

TutorialBlock::TutorialBlock(int x, int y)
    : MapBlock("新手教学区", MapBlockType::TUTORIAL, x, y, 
               "在这里学习游戏的基本操作：移动、拾取物品等") {
}

void TutorialBlock::initializeInteractionHandlers() {
    interactionHandlers_[InteractionType::PICKUP] = 
        [this](Player& player) -> InteractionResult {
            return handlePickup(player);
        };
    
    interactionHandlers_[InteractionType::ACTIVATE] = 
        [this](Player& player) -> InteractionResult {
            return handleMovement(player);
        };
}

InteractionResult TutorialBlock::handlePickup(Player& player) {
    // 给予新手物品
    auto apple = ItemFactory::createFood("苹果", FoodType::RECOVERY, Rarity::ONE_STAR);
    auto result = player.addItemToInventory(apple);
    
    if (result == InventoryResult::SUCCESS) {
        state_ = BlockState::COMPLETED;
        return InteractionResult(true, 
            "你学会了拾取物品！获得了一个苹果。\n"
            "提示：使用方向键移动，按空格键与物品交互。", 
            {apple}, true);
    }
    
    return InteractionResult(false, "背包已满，无法拾取物品");
}

InteractionResult TutorialBlock::handleMovement(Player& player) {
    return InteractionResult(true, 
        "你学会了移动！\n"
        "使用 W/A/S/D 或方向键移动角色。\n"
        "移动到有物品的地方可以拾取它们。");
}

// ==================== StatueOfSevenBlock 实现 ====================

StatueOfSevenBlock::StatueOfSevenBlock(int x, int y)
    : MapBlock("七天神像（风）", MapBlockType::STATUE_OF_SEVEN, x, y,
               "风神的七天神像，可以激活获得风元素力量，旁边还有一个宝箱"),
      activated_(false), chestOpened_(false) {
}

void StatueOfSevenBlock::initializeInteractionHandlers() {
    interactionHandlers_[InteractionType::ACTIVATE] = 
        [this](Player& player) -> InteractionResult {
            return handleActivate(player);
        };
    
    interactionHandlers_[InteractionType::PICKUP] = 
        [this](Player& player) -> InteractionResult {
            return handleChest(player);
        };
}

InteractionResult StatueOfSevenBlock::handleActivate(Player& player) {
    if (!activated_) {
        activated_ = true;
        return InteractionResult(true, 
            "你激活了七天神像！\n"
            "风元素的力量涌入你的身体，你获得了风元素能力。\n"
            "现在你可以使用风元素技能了！");
    }
    return InteractionResult(true, "七天神像已经激活过了");
}

InteractionResult StatueOfSevenBlock::handleChest(Player& player) {
    if (!chestOpened_) {
        chestOpened_ = true;
        
        // 宝箱奖励
        std::vector<std::shared_ptr<Item>> rewards;
        auto weapon = ItemFactory::createWeapon("风鹰剑", WeaponType::ONE_HANDED_SWORD, Rarity::FOUR_STAR);
        auto artifact = ItemFactory::createArtifact("翠绿之影", ArtifactType::FLOWER_OF_LIFE, Rarity::FOUR_STAR);
        auto material = ItemFactory::createMaterial("风之印", MaterialType::MONSTER_DROP, Rarity::THREE_STAR);
        
        rewards.push_back(weapon);
        rewards.push_back(artifact);
        rewards.push_back(material);
        
        // 添加到背包
        for (auto& item : rewards) {
            player.addItemToInventory(item);
        }
        
        state_ = BlockState::COMPLETED;
        return InteractionResult(true, 
            "你打开了宝箱！\n"
            "获得了：风鹰剑、翠绿之影、风之印\n"
            "这些物品将帮助你在冒险中更加强大！", 
            rewards, true);
    }
    return InteractionResult(true, "宝箱已经打开过了");
}

// ==================== SlimeBattleBlock 实现 ====================

SlimeBattleBlock::SlimeBattleBlock(int x, int y)
    : MapBlock("史莱姆栖息地", MapBlockType::BATTLE, x, y,
               "这里生活着一些史莱姆，击败它们可以获得经验和掉落物"),
      battleCompleted_(false), slimeHealth_(50), slimeAttack_(15) {
}

void SlimeBattleBlock::initializeInteractionHandlers() {
    interactionHandlers_[InteractionType::BATTLE] = 
        [this](Player& player) -> InteractionResult {
            return handleBattle(player);
        };
}

InteractionResult SlimeBattleBlock::handleBattle(Player& player) {
    if (battleCompleted_) {
        return InteractionResult(true, "史莱姆已经被击败了");
    }
    
    // 简单的战斗逻辑
    auto activeMember = player.getActiveMember();
    if (!activeMember || !activeMember->isAlive()) {
        return InteractionResult(false, "没有可战斗的角色");
    }
    
    std::string battleLog = "战斗开始！\n";
    battleLog += "史莱姆 HP: " + std::to_string(slimeHealth_) + "\n";
    battleLog += activeMember->getName() + " HP: " + std::to_string(activeMember->getCurrentHealth()) + "\n\n";
    
    // 战斗回合
    while (slimeHealth_ > 0 && activeMember->isAlive()) {
        // 玩家攻击
        int playerDamage = activeMember->getTotalAttack();
        slimeHealth_ -= playerDamage;
        battleLog += activeMember->getName() + " 对史莱姆造成了 " + std::to_string(playerDamage) + " 点伤害！\n";
        
        if (slimeHealth_ <= 0) break;
        
        // 史莱姆攻击
        int slimeDamage = slimeAttack_;
        activeMember->takeDamage(slimeDamage);
        battleLog += "史莱姆对 " + activeMember->getName() + " 造成了 " + std::to_string(slimeDamage) + " 点伤害！\n";
    }
    
    if (slimeHealth_ <= 0) {
        battleCompleted_ = true;
        state_ = BlockState::COMPLETED;
        
        // 战斗奖励
        std::vector<std::shared_ptr<Item>> rewards;
        auto slimeCondensate = ItemFactory::createMaterial("史莱姆凝液", MaterialType::MONSTER_DROP, Rarity::ONE_STAR);
        auto slimeSecretions = ItemFactory::createMaterial("史莱姆原浆", MaterialType::MONSTER_DROP, Rarity::TWO_STAR);
        
        rewards.push_back(slimeCondensate);
        rewards.push_back(slimeSecretions);
        
        for (auto& item : rewards) {
            player.addItemToInventory(item);
        }
        
        // 经验奖励
        player.experience += 50;
        
        battleLog += "\n战斗胜利！\n";
        battleLog += "获得经验：50\n";
        battleLog += "获得物品：史莱姆凝液、史莱姆原浆";
        
        return InteractionResult(true, battleLog, rewards, true);
    } else {
        battleLog += "\n战斗失败！你的角色倒下了...";
        return InteractionResult(false, battleLog);
    }
}

// ==================== AmberDialogueBlock 实现 ====================

AmberDialogueBlock::AmberDialogueBlock(int x, int y)
    : MapBlock("安伯的营地", MapBlockType::DIALOGUE, x, y,
               "西风骑士团的侦察骑士安伯在这里，她似乎有话要对你说"),
      dialogueCompleted_(false), amberJoined_(false) {
}

void AmberDialogueBlock::initializeInteractionHandlers() {
    interactionHandlers_[InteractionType::DIALOGUE] = 
        [this](Player& player) -> InteractionResult {
            return handleDialogue(player);
        };
}

InteractionResult AmberDialogueBlock::handleDialogue(Player& player) {
    if (amberJoined_) {
        return InteractionResult(true, "安伯：很高兴能和你一起冒险！");
    }
    
    if (!dialogueCompleted_) {
        dialogueCompleted_ = true;
        return InteractionResult(true, 
            "安伯：你好，旅行者！我是西风骑士团的侦察骑士安伯。\n"
            "我听说你正在寻找失散的亲人，我很乐意帮助你！\n"
            "让我加入你的队伍吧，我的弓箭技能会很有用的。");
    }
    
    // 安伯加入队伍
    amberJoined_ = true;
    player.addTeamMember("安伯", 20);
    state_ = BlockState::COMPLETED;
    
    return InteractionResult(true, 
        "安伯加入了你的队伍！\n"
        "她是一名优秀的弓箭手，擅长远程攻击。\n"
        "现在你的队伍更加强大了！", {}, true);
}

// ==================== MondstadtCityBlock 实现 ====================

MondstadtCityBlock::MondstadtCityBlock(int x, int y)
    : MapBlock("蒙德城", MapBlockType::CITY, x, y,
               "风与牧歌之城蒙德，在这里你可能会遇到新的伙伴"),
      kaeyaJoined_(false) {
}

void MondstadtCityBlock::initializeInteractionHandlers() {
    interactionHandlers_[InteractionType::ENTER] = 
        [this](Player& player) -> InteractionResult {
            return handleEnter(player);
        };
    
    interactionHandlers_[InteractionType::DIALOGUE] = 
        [this](Player& player) -> InteractionResult {
            return handleKaeyaDialogue(player);
        };
}

InteractionResult MondstadtCityBlock::handleEnter(Player& player) {
    return InteractionResult(true, 
        "欢迎来到蒙德城！\n"
        "这里是风与牧歌之城，自由之都。\n"
        "在城门口，你遇到了西风骑士团的骑兵队长凯亚。");
}

InteractionResult MondstadtCityBlock::handleKaeyaDialogue(Player& player) {
    if (kaeyaJoined_) {
        return InteractionResult(true, "凯亚：让我们继续探索吧，伙伴！");
    }
    
    kaeyaJoined_ = true;
    player.addTeamMember("凯亚", 25);
    state_ = BlockState::COMPLETED;
    
    return InteractionResult(true, 
        "凯亚：你好，旅行者！我是西风骑士团的骑兵队长凯亚。\n"
        "我听说你帮助了安伯，真是了不起！\n"
        "让我也加入你的队伍吧，我的冰元素技能会很有用的。\n\n"
        "凯亚加入了你的队伍！\n"
        "他是一名强大的冰元素剑士，擅长控制和输出。", {}, true);
}

// ==================== MapManager 实现 ====================

MapManager::MapManager() : playerX_(0), playerY_(0), 
                          minX_(0), maxX_(0), minY_(0), maxY_(0) {
    initializeMap();
}

void MapManager::initializeMap() {
    // 创建5个区块
    addBlock(std::make_shared<TutorialBlock>(0, 0));           // 教学区
    addBlock(std::make_shared<StatueOfSevenBlock>(2, 0));      // 七天神像
    addBlock(std::make_shared<SlimeBattleBlock>(1, 1));        // 史莱姆战斗
    addBlock(std::make_shared<AmberDialogueBlock>(0, 2));      // 安伯对话
    addBlock(std::make_shared<MondstadtCityBlock>(2, 2));      // 蒙德城
    
    updateMapBounds();
}

void MapManager::addBlock(std::shared_ptr<MapBlock> block) {
    blocks_.push_back(block);
    blockMap_[{block->getX(), block->getY()}] = block;
    updateMapBounds();
}

std::shared_ptr<MapBlock> MapManager::getBlock(int x, int y) const {
    auto it = blockMap_.find({x, y});
    return (it != blockMap_.end()) ? it->second : nullptr;
}

void MapManager::setPlayerPosition(int x, int y) {
    playerX_ = x;
    playerY_ = y;
}

bool MapManager::canMoveTo(int x, int y) const {
    // 检查边界
    if (x < minX_ || x > maxX_ || y < minY_ || y > maxY_) {
        return false;
    }
    
    // 检查是否有区块（目前所有位置都可以移动）
    return true;
}

bool MapManager::movePlayer(int deltaX, int deltaY) {
    int newX = playerX_ + deltaX;
    int newY = playerY_ + deltaY;
    
    if (canMoveTo(newX, newY)) {
        playerX_ = newX;
        playerY_ = newY;
        return true;
    }
    return false;
}

InteractionResult MapManager::interactWithCurrentBlock(Player& player, InteractionType interactionType) {
    auto block = getBlock(playerX_, playerY_);
    if (block && block->canInteract(interactionType)) {
        return block->interact(player, interactionType);
    }
    return InteractionResult(false, "当前位置没有可交互的内容");
}

std::vector<InteractionType> MapManager::getAvailableInteractions() const {
    auto block = getBlock(playerX_, playerY_);
    if (block) {
        return block->getAvailableInteractions();
    }
    return {};
}

std::vector<std::string> MapManager::renderMap(int width, int height) const {
    std::vector<std::string> mapLines;
    
    for (int y = minY_; y <= maxY_; ++y) {
        std::string line;
        for (int x = minX_; x <= maxX_; ++x) {
            if (x == playerX_ && y == playerY_) {
                line += "P";  // 玩家位置
            } else {
                line += getBlockSymbolAt(x, y);
            }
            line += " ";
        }
        mapLines.push_back(line);
    }
    
    return mapLines;
}

std::string MapManager::getCurrentBlockInfo() const {
    auto block = getBlock(playerX_, playerY_);
    if (block) {
        std::stringstream ss;
        ss << "位置: (" << playerX_ << ", " << playerY_ << ")\n";
        ss << "区块: " << block->getDisplayName() << "\n";
        ss << "描述: " << block->getDescription() << "\n";
        
        auto interactions = block->getAvailableInteractions();
        if (!interactions.empty()) {
            ss << "可交互: ";
            for (size_t i = 0; i < interactions.size(); ++i) {
                if (i > 0) ss << ", ";
                switch (interactions[i]) {
                    case InteractionType::PICKUP: ss << "拾取"; break;
                    case InteractionType::BATTLE: ss << "战斗"; break;
                    case InteractionType::DIALOGUE: ss << "对话"; break;
                    case InteractionType::ACTIVATE: ss << "激活"; break;
                    case InteractionType::ENTER: ss << "进入"; break;
                }
            }
        }
        return ss.str();
    }
    return "当前位置: (" + std::to_string(playerX_) + ", " + std::to_string(playerY_) + ")";
}

int MapManager::getCompletedBlocksCount() const {
    int count = 0;
    for (const auto& block : blocks_) {
        if (block->getState() == BlockState::COMPLETED) {
            count++;
        }
    }
    return count;
}

bool MapManager::isMapCompleted() const {
    return getCompletedBlocksCount() == getTotalBlocksCount();
}

void MapManager::updateMapBounds() {
    if (blocks_.empty()) return;
    
    minX_ = maxX_ = blocks_[0]->getX();
    minY_ = maxY_ = blocks_[0]->getY();
    
    for (const auto& block : blocks_) {
        minX_ = std::min(minX_, block->getX());
        maxX_ = std::max(maxX_, block->getX());
        minY_ = std::min(minY_, block->getY());
        maxY_ = std::max(maxY_, block->getY());
    }
}

std::string MapManager::getBlockSymbolAt(int x, int y) const {
    auto block = getBlock(x, y);
    return block ? block->getDisplaySymbol() : ".";
}
