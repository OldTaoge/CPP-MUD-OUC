#include "map_v2.h"
#include <iostream>
#include <algorithm>
#include <sstream>

// ==================== MapBlock 基类实现 ====================

MapBlock::MapBlock(int id, const std::string& name, MapBlockType type, const std::string& description)
    : id_(id), name_(name), type_(type), description_(description), 
      state_(BlockState::UNLOCKED), playerX_(4), playerY_(4) {
    // 初始化网格为空
    for (int y = 0; y < BLOCK_SIZE; ++y) {
        for (int x = 0; x < BLOCK_SIZE; ++x) {
            grid_[y][x] = MapCell(CellType::EMPTY, ".", "空地");
        }
    }
}

MapCell& MapBlock::getCell(int x, int y) {
    if (!isValidPosition(x, y)) {
        static MapCell invalidCell(CellType::WALL, "#", "墙壁");
        return invalidCell;
    }
    return grid_[y][x];
}

const MapCell& MapBlock::getCell(int x, int y) const {
    if (!isValidPosition(x, y)) {
        static MapCell invalidCell(CellType::WALL, "#", "墙壁");
        return invalidCell;
    }
    return grid_[y][x];
}

void MapBlock::setCell(int x, int y, const MapCell& cell) {
    if (isValidPosition(x, y)) {
        grid_[y][x] = cell;
    }
}

bool MapBlock::isValidPosition(int x, int y) const {
    return x >= 0 && x < BLOCK_SIZE && y >= 0 && y < BLOCK_SIZE;
}

void MapBlock::setPlayerPosition(int x, int y) {
    if (isValidPosition(x, y)) {
        playerX_ = x;
        playerY_ = y;
    }
}

bool MapBlock::canMoveTo(int x, int y) const {
    if (!isValidPosition(x, y)) {
        return false;
    }
    
    const MapCell& cell = getCell(x, y);
    return cell.type != CellType::WALL;
}

bool MapBlock::movePlayer(int deltaX, int deltaY) {
    int newX = playerX_ + deltaX;
    int newY = playerY_ + deltaY;
    
    if (canMoveTo(newX, newY)) {
        playerX_ = newX;
        playerY_ = newY;
        return true;
    }
    return false;
}

bool MapBlock::isExit(int x, int y) const {
    if (!isValidPosition(x, y)) return false;
    
    CellType type = getCell(x, y).type;
    return type == CellType::EXIT_NORTH || type == CellType::EXIT_SOUTH ||
           type == CellType::EXIT_EAST || type == CellType::EXIT_WEST;
}

int MapBlock::getExitTarget(int x, int y) const {
    auto it = exits_.find({x, y});
    return (it != exits_.end()) ? it->second : -1;
}

InteractionResult MapBlock::interact(Player& player, InteractionType interactionType) {
    auto it = interactionHandlers_.find(interactionType);
    if (it != interactionHandlers_.end()) {
        return it->second(player, playerX_, playerY_);
    }
    return InteractionResult(false, "无法进行此交互");
}

std::vector<InteractionType> MapBlock::getAvailableInteractions(int x, int y) const {
    if (!isValidPosition(x, y)) {
        return {};
    }
    
    return getCell(x, y).interactions;
}

std::vector<std::string> MapBlock::render() const {
    std::vector<std::string> result;
    
    // 直接渲染地图网格 - 使用等宽ASCII字符（区块名称将在标题栏显示）
    result.push_back("+---+---+---+---+---+");
    for (int y = 0; y < BLOCK_SIZE; ++y) {
        std::string line = "|";
        for (int x = 0; x < BLOCK_SIZE; ++x) {
            if (x == playerX_ && y == playerY_) {
                line += " P ";  // 玩家位置 - 使用等宽字符
            } else {
                // 根据地形类型使用不同的等宽符号
                const MapCell& cell = getCell(x, y);
                switch (cell.type) {
                    case CellType::EMPTY: line += " . "; break;
                    case CellType::WALL: line += " # "; break;
                    case CellType::ITEM: line += " I "; break;
                    case CellType::NPC: line += " N "; break;
                    case CellType::STATUE: line += " S "; break;
                    case CellType::MONSTER: line += " M "; break;
                    case CellType::EXIT_NORTH: line += " ^ "; break;
                    case CellType::EXIT_SOUTH: line += " v "; break;
                    case CellType::EXIT_EAST: line += " > "; break;
                    case CellType::EXIT_WEST: line += " < "; break;
                    default: line += " ? "; break;
                }
            }
            line += "|";
        }
        result.push_back(line);
        if (y < BLOCK_SIZE - 1) {
            result.push_back("+---+---+---+---+---+");
        }
    }
    result.push_back("+---+---+---+---+---+");
    
    // 添加图例说明 - 包含高亮提示
    result.push_back("");
    result.push_back("=== 地图图例 ===");
    result.push_back("P = 玩家位置 (黄色高亮)");
    result.push_back("I = 物品/宝箱 (绿色高亮) - 可拾取");
    result.push_back("N = NPC角色 (蓝色高亮) - 可对话");
    result.push_back("S = 七天神像 (金色高亮) - 可激活");
    result.push_back("M = 怪物敌人 (红色高亮) - 可战斗");
    result.push_back("^v>< = 区域出口 (紫色高亮) - 可传送");
    result.push_back("# = 墙壁障碍 (灰色)");
    result.push_back(". = 空地 (白色)");
    
    return result;
}

std::string MapBlock::getCurrentCellInfo() const {
    std::stringstream ss;
    ss << "区块: " << name_ << "\n";
    ss << "位置: (" << playerX_ << ", " << playerY_ << ")\n";
    
    const MapCell& cell = getCell(playerX_, playerY_);
    ss << "地形: " << cell.description << "\n";
    
    if (!cell.interactions.empty()) {
        ss << "可交互操作: ";
        for (size_t i = 0; i < cell.interactions.size(); ++i) {
            if (i > 0) ss << ", ";
            switch (cell.interactions[i]) {
                case InteractionType::PICKUP: ss << "[拾取]"; break;
                case InteractionType::BATTLE: ss << "[战斗]"; break;
                case InteractionType::DIALOGUE: ss << "[对话]"; break;
                case InteractionType::ACTIVATE: ss << "[激活]"; break;
                case InteractionType::ENTER: ss << "[进入]"; break;
            }
        }
        ss << "\n按空格键进行交互";
    } else {
        ss << "无交互选项";
    }
    
    return ss.str();
}

// ==================== TutorialBlock 实现 ====================

TutorialBlock::TutorialBlock() : MapBlock(0, "新手教学区", MapBlockType::TUTORIAL, 
    "在这里学习游戏的基本操作：移动、拾取物品等") {
    initializeGrid();
    initializeInteractionHandlers();
    initializeExits();
}

void TutorialBlock::initializeGrid() {
    // 创建边界墙
    for (int i = 0; i < BLOCK_SIZE; ++i) {
        setCell(i, 0, MapCell(CellType::WALL, "#", "墙壁"));
        setCell(i, BLOCK_SIZE-1, MapCell(CellType::WALL, "#", "墙壁"));
        setCell(0, i, MapCell(CellType::WALL, "#", "墙壁"));
        setCell(BLOCK_SIZE-1, i, MapCell(CellType::WALL, "#", "墙壁"));
    }
    
    // 放置一些物品
    MapCell itemCell(CellType::ITEM, "I", "苹果");
    itemCell.interactions.push_back(InteractionType::PICKUP);
    itemCell.item = ItemFactory::createFood("苹果", FoodType::RECOVERY, Rarity::ONE_STAR);
    setCell(3, 3, itemCell);
    
    // 设置东出口
    MapCell exitCell(CellType::EXIT_EAST, ">", "通往七天神像");
    setCell(BLOCK_SIZE-1, 4, exitCell);
    exits_[{BLOCK_SIZE-1, 4}] = 1;  // 连接到区块1
}

void TutorialBlock::initializeInteractionHandlers() {
    interactionHandlers_[InteractionType::PICKUP] = 
        [this](Player& player, int x, int y) -> InteractionResult {
            return handlePickup(player, x, y);
        };
}

void TutorialBlock::initializeExits() {
    // 出口已在initializeGrid中设置
}

InteractionResult TutorialBlock::handlePickup(Player& player, int x, int y) {
    MapCell& cell = getCell(x, y);
    if (cell.type == CellType::ITEM && cell.item) {
        auto result = player.addItemToInventory(cell.item);
        
        if (result == InventoryResult::SUCCESS) {
            // 移除物品
            cell.type = CellType::EMPTY;
            cell.symbol = ".";
            cell.description = "空地";
            cell.interactions.clear();
            cell.item = nullptr;
            
            state_ = BlockState::COMPLETED;
            return InteractionResult(true, 
                "你学会了拾取物品！获得了一个苹果。\n"
                "提示：使用方向键移动，按空格键与物品交互。\n"
                "现在可以前往东边的出口继续冒险！", 
                {cell.item}, true);
        }
    }
    
    return InteractionResult(false, "这里没有可拾取的物品");
}

InteractionResult TutorialBlock::handleMovement(Player& player, int x, int y) {
    return InteractionResult(true, 
        "你学会了移动！\n"
        "使用方向键移动角色。\n"
        "移动到有物品的地方可以拾取它们。");
}

// ==================== StatueOfSevenBlock 实现 ====================

StatueOfSevenBlock::StatueOfSevenBlock() 
    : MapBlock(1, "七天神像（风）", MapBlockType::STATUE_OF_SEVEN,
               "风神的七天神像，可以激活获得风元素力量，旁边还有一个宝箱"),
      activated_(false), chestOpened_(false) {
    initializeGrid();
    initializeInteractionHandlers();
    initializeExits();
}

void StatueOfSevenBlock::initializeGrid() {
    // 创建边界墙
    for (int i = 0; i < BLOCK_SIZE; ++i) {
        setCell(i, 0, MapCell(CellType::WALL, "#", "墙壁"));
        setCell(i, BLOCK_SIZE-1, MapCell(CellType::WALL, "#", "墙壁"));
        setCell(0, i, MapCell(CellType::WALL, "#", "墙壁"));
        setCell(BLOCK_SIZE-1, i, MapCell(CellType::WALL, "#", "墙壁"));
    }
    
    // 放置七天神像
    MapCell statueCell(CellType::STATUE, "S", "七天神像（风）");
    statueCell.interactions.push_back(InteractionType::ACTIVATE);
    setCell(4, 3, statueCell);
    
    // 放置宝箱
    MapCell chestCell(CellType::ITEM, "C", "宝箱");
    chestCell.interactions.push_back(InteractionType::PICKUP);
    setCell(6, 3, chestCell);
    
    // 设置西出口（回到教学区）
    MapCell westExit(CellType::EXIT_WEST, "<", "返回教学区");
    setCell(0, 4, westExit);
    exits_[{0, 4}] = 0;
    
    // 设置南出口（前往史莱姆区）
    MapCell southExit(CellType::EXIT_SOUTH, "v", "前往史莱姆栖息地");
    setCell(4, BLOCK_SIZE-1, southExit);
    exits_[{4, BLOCK_SIZE-1}] = 2;
}

void StatueOfSevenBlock::initializeInteractionHandlers() {
    interactionHandlers_[InteractionType::ACTIVATE] = 
        [this](Player& player, int x, int y) -> InteractionResult {
            return handleActivate(player, x, y);
        };
    
    interactionHandlers_[InteractionType::PICKUP] = 
        [this](Player& player, int x, int y) -> InteractionResult {
            return handleChest(player, x, y);
        };
}

void StatueOfSevenBlock::initializeExits() {
    // 出口已在initializeGrid中设置
}

InteractionResult StatueOfSevenBlock::handleActivate(Player& player, int x, int y) {
    if (!activated_) {
        activated_ = true;
        return InteractionResult(true, 
            "你激活了七天神像！\n"
            "风元素的力量涌入你的身体，你获得了风元素能力。\n"
            "现在你可以使用风元素技能了！");
    }
    return InteractionResult(true, "七天神像已经激活过了");
}

InteractionResult StatueOfSevenBlock::handleChest(Player& player, int x, int y) {
    MapCell& cell = getCell(x, y);
    if (cell.type == CellType::ITEM && !chestOpened_) {
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
        
        // 移除宝箱
        cell.type = CellType::EMPTY;
        cell.symbol = ".";
        cell.description = "空地";
        cell.interactions.clear();
        
        if (activated_) {
            state_ = BlockState::COMPLETED;
        }
        
        return InteractionResult(true, 
            "你打开了宝箱！\n"
            "获得了：风鹰剑、翠绿之影、风之印\n"
            "这些物品将帮助你在冒险中更加强大！", 
            rewards, activated_);
    }
    return InteractionResult(true, "宝箱已经打开过了");
}

// ==================== SlimeBattleBlock 实现 ====================

SlimeBattleBlock::SlimeBattleBlock()
    : MapBlock(2, "史莱姆栖息地", MapBlockType::BATTLE,
               "这里生活着一些史莱姆，击败它们可以获得经验和掉落物"),
      battleCompleted_(false), slimeHealth_(50), slimeAttack_(15) {
    initializeGrid();
    initializeInteractionHandlers();
    initializeExits();
}

void SlimeBattleBlock::initializeGrid() {
    // 创建边界墙
    for (int i = 0; i < BLOCK_SIZE; ++i) {
        setCell(i, 0, MapCell(CellType::WALL, "#", "墙壁"));
        setCell(i, BLOCK_SIZE-1, MapCell(CellType::WALL, "#", "墙壁"));
        setCell(0, i, MapCell(CellType::WALL, "#", "墙壁"));
        setCell(BLOCK_SIZE-1, i, MapCell(CellType::WALL, "#", "墙壁"));
    }
    
    // 放置史莱姆
    MapCell slimeCell(CellType::MONSTER, "M", "史莱姆");
    slimeCell.interactions.push_back(InteractionType::BATTLE);
    setCell(5, 5, slimeCell);
    
    // 设置北出口（回到七天神像）
    MapCell northExit(CellType::EXIT_NORTH, "^", "返回七天神像");
    setCell(4, 0, northExit);
    exits_[{4, 0}] = 1;
    
    // 设置东出口（前往安伯营地）
    MapCell eastExit(CellType::EXIT_EAST, ">", "前往安伯营地");
    setCell(BLOCK_SIZE-1, 4, eastExit);
    exits_[{BLOCK_SIZE-1, 4}] = 3;
}

void SlimeBattleBlock::initializeInteractionHandlers() {
    interactionHandlers_[InteractionType::BATTLE] = 
        [this](Player& player, int x, int y) -> InteractionResult {
            return handleBattle(player, x, y);
        };
}

void SlimeBattleBlock::initializeExits() {
    // 出口已在initializeGrid中设置
}

InteractionResult SlimeBattleBlock::handleBattle(Player& player, int x, int y) {
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
        
        // 移除史莱姆
        MapCell& cell = getCell(x, y);
        cell.type = CellType::EMPTY;
        cell.symbol = ".";
        cell.description = "空地";
        cell.interactions.clear();
        
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

AmberDialogueBlock::AmberDialogueBlock()
    : MapBlock(3, "安伯的营地", MapBlockType::DIALOGUE,
               "西风骑士团的侦察骑士安伯在这里，她似乎有话要对你说"),
      dialogueCompleted_(false), amberJoined_(false) {
    initializeGrid();
    initializeInteractionHandlers();
    initializeExits();
}

void AmberDialogueBlock::initializeGrid() {
    // 创建边界墙
    for (int i = 0; i < BLOCK_SIZE; ++i) {
        setCell(i, 0, MapCell(CellType::WALL, "#", "墙壁"));
        setCell(i, BLOCK_SIZE-1, MapCell(CellType::WALL, "#", "墙壁"));
        setCell(0, i, MapCell(CellType::WALL, "#", "墙壁"));
        setCell(BLOCK_SIZE-1, i, MapCell(CellType::WALL, "#", "墙壁"));
    }
    
    // 放置安伯
    MapCell amberCell(CellType::NPC, "A", "安伯");
    amberCell.interactions.push_back(InteractionType::DIALOGUE);
    setCell(6, 4, amberCell);
    
    // 设置西出口（回到史莱姆区）
    MapCell westExit(CellType::EXIT_WEST, "<", "返回史莱姆栖息地");
    setCell(0, 4, westExit);
    exits_[{0, 4}] = 2;
    
    // 设置南出口（前往蒙德城）
    MapCell southExit(CellType::EXIT_SOUTH, "v", "前往蒙德城");
    setCell(4, BLOCK_SIZE-1, southExit);
    exits_[{4, BLOCK_SIZE-1}] = 4;
}

void AmberDialogueBlock::initializeInteractionHandlers() {
    interactionHandlers_[InteractionType::DIALOGUE] = 
        [this](Player& player, int x, int y) -> InteractionResult {
            return handleDialogue(player, x, y);
        };
}

void AmberDialogueBlock::initializeExits() {
    // 出口已在initializeGrid中设置
}

InteractionResult AmberDialogueBlock::handleDialogue(Player& player, int x, int y) {
    if (amberJoined_) {
        return InteractionResult(true, "安伯：很高兴能和你一起冒险！");
    }
    // 第一次交互直接加入队伍
    amberJoined_ = true;
    player.addTeamMember("安伯", 20);
    state_ = BlockState::COMPLETED;
    return InteractionResult(true,
        "安伯：你好，旅行者！我是西风骑士团的侦察骑士安伯。\n"
        "我听说你正在寻找失散的亲人，我很乐意帮助你！\n"
        "安伯加入了你的队伍！\n"
        "她是一名优秀的弓箭手，擅长远程攻击。\n"
        "现在你的队伍更加强大了！", {}, true);
}

// ==================== MondstadtCityBlock 实现 ====================

MondstadtCityBlock::MondstadtCityBlock()
    : MapBlock(4, "蒙德城", MapBlockType::CITY,
               "风与牧歌之城蒙德，在这里你可能会遇到新的伙伴"),
      kaeyaJoined_(false) {
    initializeGrid();
    initializeInteractionHandlers();
    initializeExits();
}

void MondstadtCityBlock::initializeGrid() {
    // 创建边界墙
    for (int i = 0; i < BLOCK_SIZE; ++i) {
        setCell(i, 0, MapCell(CellType::WALL, "#", "墙壁"));
        setCell(i, BLOCK_SIZE-1, MapCell(CellType::WALL, "#", "墙壁"));
        setCell(0, i, MapCell(CellType::WALL, "#", "墙壁"));
        setCell(BLOCK_SIZE-1, i, MapCell(CellType::WALL, "#", "墙壁"));
    }
    
    // 放置凯亚
    MapCell kaeyaCell(CellType::NPC, "K", "凯亚");
    kaeyaCell.interactions.push_back(InteractionType::DIALOGUE);
    setCell(5, 6, kaeyaCell);
    
    // 设置北出口（回到安伯营地）
    MapCell northExit(CellType::EXIT_NORTH, "^", "返回安伯营地");
    setCell(4, 0, northExit);
    exits_[{4, 0}] = 3;
}

void MondstadtCityBlock::initializeInteractionHandlers() {
    interactionHandlers_[InteractionType::DIALOGUE] = 
        [this](Player& player, int x, int y) -> InteractionResult {
            return handleKaeyaDialogue(player, x, y);
        };
}

void MondstadtCityBlock::initializeExits() {
    // 出口已在initializeGrid中设置
}

InteractionResult MondstadtCityBlock::handleEnter(Player& player, int x, int y) {
    return InteractionResult(true, 
        "欢迎来到蒙德城！\n"
        "这里是风与牧歌之城，自由之都。\n"
        "在城里，你遇到了西风骑士团的骑兵队长凯亚。");
}

InteractionResult MondstadtCityBlock::handleKaeyaDialogue(Player& player, int x, int y) {
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

// ==================== MapManagerV2 实现 ====================

MapManagerV2::MapManagerV2() : currentBlockId_(0) {
    initializeMap();
}

void MapManagerV2::initializeMap() {
    // 创建5个区块
    addBlock(std::make_shared<TutorialBlock>());           // 区块0: 教学区
    addBlock(std::make_shared<StatueOfSevenBlock>());      // 区块1: 七天神像
    addBlock(std::make_shared<SlimeBattleBlock>());        // 区块2: 史莱姆战斗
    addBlock(std::make_shared<AmberDialogueBlock>());      // 区块3: 安伯对话
    addBlock(std::make_shared<MondstadtCityBlock>());      // 区块4: 蒙德城
}

void MapManagerV2::addBlock(std::shared_ptr<MapBlock> block) {
    blocks_[block->getId()] = block;
}

std::shared_ptr<MapBlock> MapManagerV2::getBlock(int blockId) const {
    auto it = blocks_.find(blockId);
    return (it != blocks_.end()) ? it->second : nullptr;
}

std::shared_ptr<MapBlock> MapManagerV2::getCurrentBlock() const {
    return getBlock(currentBlockId_);
}

bool MapManagerV2::switchToBlock(int blockId, int startX, int startY) {
    auto block = getBlock(blockId);
    if (block) {
        currentBlockId_ = blockId;
        block->setPlayerPosition(startX, startY);
        return true;
    }
    return false;
}

bool MapManagerV2::movePlayer(int deltaX, int deltaY) {
    auto currentBlock = getCurrentBlock();
    if (!currentBlock) return false;
    
    auto pos = currentBlock->getPlayerPosition();
    int newX = pos.first + deltaX;
    int newY = pos.second + deltaY;
    
    // 检查是否是出口
    if (currentBlock->isExit(newX, newY)) {
        int targetBlockId = currentBlock->getExitTarget(newX, newY);
        if (targetBlockId >= 0) {
            handleBlockTransition(targetBlockId);
            return true;
        }
    }
    
    return currentBlock->movePlayer(deltaX, deltaY);
}

std::pair<int, int> MapManagerV2::getPlayerPosition() const {
    auto currentBlock = getCurrentBlock();
    if (currentBlock) {
        return currentBlock->getPlayerPosition();
    }
    return {0, 0};
}

InteractionResult MapManagerV2::interactWithCurrentCell(Player& player, InteractionType interactionType) {
    auto currentBlock = getCurrentBlock();
    if (currentBlock) {
        return currentBlock->interact(player, interactionType);
    }
    return InteractionResult(false, "当前区块不存在");
}

std::vector<InteractionType> MapManagerV2::getAvailableInteractions() const {
    auto currentBlock = getCurrentBlock();
    if (currentBlock) {
        auto pos = currentBlock->getPlayerPosition();
        return currentBlock->getAvailableInteractions(pos.first, pos.second);
    }
    return {};
}

std::vector<std::string> MapManagerV2::renderCurrentBlock() const {
    auto currentBlock = getCurrentBlock();
    if (currentBlock) {
        return currentBlock->render();
    }
    return {"当前区块不存在"};
}

std::vector<std::string> MapManagerV2::renderFullMap() const {
    std::vector<std::string> fullMap;
    
    // 添加标题
    fullMap.push_back("=== 完整地图总览 ===");
    fullMap.push_back("");
    
    // 显示区块连接图
    fullMap.push_back("区块连接关系:");
    fullMap.push_back("[0教学区] → [1神像] → [2史莱姆] → [3安伯] → [4蒙德城]");
    fullMap.push_back("");
    
    // 显示每个区块的状态
    for (const auto& pair : blocks_) {
        int blockId = pair.first;
        auto block = pair.second;
        
        std::string statusLine = "区块" + std::to_string(blockId) + ": " + block->getName();
        
        // 添加状态标记
        switch (block->getState()) {
            case BlockState::LOCKED:
                statusLine += " [锁定]";
                break;
            case BlockState::UNLOCKED:
                statusLine += " [可探索]";
                break;
            case BlockState::COMPLETED:
                statusLine += " [已完成]";
                break;
        }
        
        // 标记当前区块
        if (blockId == currentBlockId_) {
            statusLine += " ← 当前位置";
        }
        
        fullMap.push_back(statusLine);
        fullMap.push_back("  描述: " + block->getDescription());
        fullMap.push_back("");
    }
    
    // 添加进度信息
    int completed = getCompletedBlocksCount();
    int total = getTotalBlocksCount();
    fullMap.push_back("探索进度: " + std::to_string(completed) + "/" + std::to_string(total) + 
                     " (" + std::to_string((completed * 100) / total) + "%)");
    
    // 添加图例
    fullMap.push_back("");
    fullMap.push_back("图例说明:");
    fullMap.push_back("  [锁定] - 尚未解锁的区块");
    fullMap.push_back("  [可探索] - 可以进入但未完成的区块");
    fullMap.push_back("  [已完成] - 已完成所有任务的区块");
    
    return fullMap;
}

std::string MapManagerV2::getCurrentCellInfo() const {
    auto currentBlock = getCurrentBlock();
    if (currentBlock) {
        return currentBlock->getCurrentCellInfo();
    }
    return "当前区块不存在";
}

std::string MapManagerV2::getBlockInfo() const {
    auto currentBlock = getCurrentBlock();
    if (currentBlock) {
        std::stringstream ss;
        ss << "当前区块: " << currentBlock->getName() << " (ID: " << currentBlock->getId() << ")\n";
        ss << "描述: " << currentBlock->getDescription() << "\n";
        ss << "状态: ";
        switch (currentBlock->getState()) {
            case BlockState::LOCKED: ss << "锁定"; break;
            case BlockState::UNLOCKED: ss << "解锁"; break;
            case BlockState::COMPLETED: ss << "完成"; break;
        }
        return ss.str();
    }
    return "当前区块不存在";
}

int MapManagerV2::getCompletedBlocksCount() const {
    int count = 0;
    for (const auto& pair : blocks_) {
        if (pair.second->getState() == BlockState::COMPLETED) {
            count++;
        }
    }
    return count;
}

bool MapManagerV2::isMapCompleted() const {
    return getCompletedBlocksCount() == getTotalBlocksCount();
}

void MapManagerV2::handleBlockTransition(int targetBlockId) {
    auto targetBlock = getBlock(targetBlockId);
    if (targetBlock) {
        currentBlockId_ = targetBlockId;
        
        // 根据来源方向设置玩家起始位置
        int startX = 4, startY = 4;  // 默认中心位置
        
        // 这里可以根据具体的出入口逻辑调整起始位置
        // 例如：从北边进入则放在南边，从南边进入则放在北边等
        
        targetBlock->setPlayerPosition(startX, startY);
    }
}
