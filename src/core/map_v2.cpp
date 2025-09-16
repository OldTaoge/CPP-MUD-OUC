// =============================================
// 文件: map_v2.cpp
// 描述: 地图系统实现。区块网格渲染、交互处理、区块切换与总览输出。
// =============================================
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
    // 使用更清晰的边框字符，确保在各种终端中都能正确显示
    result.push_back("+---+---+---+---+---+---+---+---+---+");
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
            result.push_back("+---+---+---+---+---+---+---+---+---+");
        }
    }
    result.push_back("+---+---+---+---+---+---+---+---+---+");
    
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
                case InteractionType::SHOP: ss << "[商店]"; break;
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
        // 若宝箱已开，则两项条件均满足，标记区块完成
        if (chestOpened_) {
            state_ = BlockState::COMPLETED;
        }
        return InteractionResult(true, 
            "你激活了七天神像！\n"
            "风元素的力量涌入你的身体，你获得了风元素能力。\n"
            "现在你可以使用风元素技能了！", {}, state_ == BlockState::COMPLETED);
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
    
    // 设置东出口（前往安安柏营地）
    MapCell eastExit(CellType::EXIT_EAST, ">", "前往安安柏营地");
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
    : MapBlock(3, "安柏的营地", MapBlockType::DIALOGUE,
               "西风骑士团的侦察骑士安柏在这里，她似乎有话要对你说"),
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
    
    // 放置安柏
    MapCell amberCell(CellType::NPC, "A", "安柏");
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
        return InteractionResult(true, "安柏：很高兴能和你一起冒险！");
    }
    // 第一次交互直接加入队伍
    amberJoined_ = true;
    player.addTeamMember("安柏", 20);
    state_ = BlockState::COMPLETED;
    return InteractionResult(true,
        "安柏：你好，旅行者！我是西风骑士团的侦察骑士安柏。\n"
        "我听说你正在寻找失散的亲人，我很乐意帮助你！\n"
        "安柏加入了你的队伍！\n"
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

    // 放置商人 NPC（符号 S）
    MapCell shopCell(CellType::NPC, "S", "商人");
    shopCell.interactions.push_back(InteractionType::SHOP);
    setCell(3, 5, shopCell);
    
    // 设置北出口（回到安柏营地）
    MapCell northExit(CellType::EXIT_NORTH, "^", "返回安柏营地");
    setCell(4, 0, northExit);
    exits_[{4, 0}] = 3;
}

void MondstadtCityBlock::initializeInteractionHandlers() {
    interactionHandlers_[InteractionType::DIALOGUE] = 
        [this](Player& player, int x, int y) -> InteractionResult {
            return handleKaeyaDialogue(player, x, y);
        };

    interactionHandlers_[InteractionType::SHOP] =
        [this](Player& player, int x, int y) -> InteractionResult {
            // 使用 InteractionResult 的 shouldChangeBlock 作为界面切换信号不合适，改用成功+特定消息标记
            return InteractionResult(true, "OPEN_SHOP");
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
        "我听说你帮助了安柏，真是了不起！\n"
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
    addBlock(std::make_shared<AmberDialogueBlock>());      // 区块3: 安柏对话
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
    fullMap.push_back("[0教学区] → [1神像] → [2史莱姆] → [3安柏] → [4蒙德城]");
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

// 以当前区块为中心，显示上下左右与之相邻的区块，仅显示区块名以表达位置关系
std::vector<std::string> MapManagerV2::renderStitchedBlocks(int radius) const {
    (void)radius; // 目前半径未使用，展示所有可达区块

    auto center = getCurrentBlock();
    if (!center) return {"当前区块不存在"};

    // 通过扫描每个区块的出口，构建邻接关系
    std::map<int, std::map<char, int>> adj; // id -> {dir -> neighborId}
    auto add_edge = [&](int a, char dir, int b) {
        adj[a][dir] = b;
        // 补充逆向边
        char rev = 'X';
        if (dir == 'N') rev = 'S'; else if (dir == 'S') rev = 'N';
        else if (dir == 'E') rev = 'W'; else if (dir == 'W') rev = 'E';
        if (rev != 'X') {
            // 只在未存在时写入
            if (adj[b].find(rev) == adj[b].end()) adj[b][rev] = a;
        }
    };

    for (const auto& kv : blocks_) {
        int id = kv.first;
        auto blk = kv.second;
        if (!blk) continue;
        for (int y = 0; y < MapBlock::BLOCK_SIZE; ++y) {
            for (int x = 0; x < MapBlock::BLOCK_SIZE; ++x) {
                if (!blk->isExit(x, y)) continue;
                int tgt = blk->getExitTarget(x, y);
                if (tgt < 0) continue;
                if (blocks_.find(tgt) == blocks_.end()) continue;
                auto ct = blk->getCell(x, y).type;
                switch (ct) {
                    case CellType::EXIT_NORTH: add_edge(id, 'N', tgt); break;
                    case CellType::EXIT_SOUTH: add_edge(id, 'S', tgt); break;
                    case CellType::EXIT_EAST:  add_edge(id, 'E', tgt); break;
                    case CellType::EXIT_WEST:  add_edge(id, 'W', tgt); break;
                    default: break;
                }
            }
        }
    }

    // BFS 放置，当前为 (0,0)，放置所有可达区块
    struct Pt { int x; int y; };
    std::map<int, Pt> pos; // blockId -> 坐标
    std::map<int, bool> vis;
    std::vector<int> q;
    int cId = currentBlockId_;
    pos[cId] = {0, 0}; vis[cId] = true; q.push_back(cId);

    auto step = [](const Pt& p, char dir) -> Pt {
        if (dir == 'N') return {p.x, p.y - 1};
        if (dir == 'S') return {p.x, p.y + 1};
        if (dir == 'W') return {p.x - 1, p.y};
        return {p.x + 1, p.y}; // 'E'
    };

    for (size_t i = 0; i < q.size(); ++i) {
        int u = q[i];
        auto it = adj.find(u);
        if (it == adj.end()) continue;
        for (const auto& kv2 : it->second) {
            char dir = kv2.first;
            int v = kv2.second;
            if (vis[v]) continue;
            if (!getBlock(v)) continue;
            pos[v] = step(pos[u], dir);
            vis[v] = true;
            q.push_back(v);
        }
    }

    // 生成网格（仅显示名字），统一每格宽度
    int minX = 0, maxX = 0, minY = 0, maxY = 0;
    for (const auto& p : pos) {
        minX = std::min(minX, p.second.x);
        maxX = std::max(maxX, p.second.x);
        minY = std::min(minY, p.second.y);
        maxY = std::max(maxY, p.second.y);
    }

    // 名称映射与最大宽
    std::map<int, std::string> names;
    size_t cellW = 0;
    for (const auto& p : pos) {
        auto b = getBlock(p.first);
        std::string label = b ? b->getName() : "";
        names[p.first] = label;
        cellW = std::max(cellW, label.size());
    }
    // 适度留白
    cellW = std::max<size_t>(cellW, 4);

    auto centerMark = "*"; // 标记当前区块

    auto padCenter = [](const std::string& s, size_t w) {
        if (s.size() >= w) return s.substr(0, w);
        size_t left = (w - s.size()) / 2;
        size_t right = w - s.size() - left;
        return std::string(left, ' ') + s + std::string(right, ' ');
    };

    std::vector<std::string> out;
    out.push_back("=== 拼接区块（名称） ===");
    out.push_back("");

    // 渲染按行输出，隐藏无内容格，但保持基本对齐

    for (int y = minY; y <= maxY; ++y) {
        // 若不是第一行，输出垂直连接线
        if (y > minY) {
            std::string connLine;
            for (int x = minX; x <= maxX; ++x) {
                // 上下都有块且有连接则画竖线
                int upId = -1, downId = -1;
                for (const auto& kvp : pos) {
                    if (kvp.second.x == x && kvp.second.y == y - 1) upId = kvp.first;
                    if (kvp.second.x == x && kvp.second.y == y) downId = kvp.first;
                }
                bool has = false;
                if (upId >= 0 && downId >= 0) {
                    auto itUp = adj.find(upId);
                    if (itUp != adj.end()) {
                        auto itN = itUp->second.find('S');
                        has = (itN != itUp->second.end() && itN->second == downId);
                    }
                }
                connLine += has ? padCenter("|", cellW) : std::string(cellW, ' ');
                if (x < maxX) connLine += "   "; // 横向间隔
            }
            // 仅当存在至少一个连接时输出此行
            if (connLine.find('|') != std::string::npos) out.push_back(connLine);
        }

        // 名称行和横向连线
        std::string nameLine;
        for (int x = minX; x <= maxX; ++x) {
            int idHere = -1;
            for (const auto& kvp : pos) {
                if (kvp.second.x == x && kvp.second.y == y) { idHere = kvp.first; break; }
            }
            std::string label;
            if (idHere >= 0) {
                label = names[idHere];
                if (idHere == cId) label += centerMark;
                nameLine += padCenter(label, cellW);
            } else {
                nameLine += std::string(cellW, ' ');
            }

            // 横向连接符（到右侧）
            if (x < maxX) {
                int rightId = -1;
                for (const auto& kvp : pos) {
                    if (kvp.second.x == x + 1 && kvp.second.y == y) { rightId = kvp.first; break; }
                }
                bool has = false;
                if (idHere >= 0 && rightId >= 0) {
                    auto itAdj = adj.find(idHere);
                    if (itAdj != adj.end()) {
                        auto itE = itAdj->second.find('E');
                        has = (itE != itAdj->second.end() && itE->second == rightId);
                    }
                }
                nameLine += has ? " - " : "   ";
            }
        }
        // 如果整行为空（没有任何名称字符），则跳过
        bool anyChar = false;
        for (char ch : nameLine) { if (ch != ' ') { anyChar = true; break; } }
        if (anyChar) out.push_back(nameLine);
    }

    out.push_back("");
    out.push_back("* 为当前位置");
    return out;
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
