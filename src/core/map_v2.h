#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include "../core/item.h"
#include "../player/player.h"

// 地图区块类型枚举
enum class MapBlockType {
    TUTORIAL,           // 教学区块
    STATUE_OF_SEVEN,    // 七天神像
    BATTLE,             // 战斗区块
    DIALOGUE,           // 对话区块
    CITY                // 城市区块
};

// 地图区块状态
enum class BlockState {
    LOCKED,     // 锁定状态
    UNLOCKED,   // 解锁状态
    COMPLETED   // 完成状态
};

// 交互类型
enum class InteractionType {
    PICKUP,     // 拾取物品
    BATTLE,     // 战斗
    DIALOGUE,   // 对话
    ACTIVATE,   // 激活
    ENTER       // 进入
};

// 地图单元格类型
enum class CellType {
    EMPTY,      // 空地
    WALL,       // 墙壁
    PLAYER,     // 玩家
    ITEM,       // 物品
    NPC,        // NPC
    STATUE,     // 神像
    MONSTER,    // 怪物
    EXIT_NORTH, // 北出口
    EXIT_SOUTH, // 南出口
    EXIT_EAST,  // 东出口
    EXIT_WEST   // 西出口
};

// 前向声明
class MapBlock;
class MapManagerV2;

// 交互结果结构
struct InteractionResult {
    bool success;
    std::string message;
    std::vector<std::shared_ptr<Item>> rewards;
    bool blockCompleted;
    bool shouldChangeBlock;
    int targetBlockId;
    
    InteractionResult(bool s = false, const std::string& msg = "", 
                     const std::vector<std::shared_ptr<Item>>& r = {},
                     bool completed = false, bool changeBlock = false, int targetId = -1)
        : success(s), message(msg), rewards(r), blockCompleted(completed), 
          shouldChangeBlock(changeBlock), targetBlockId(targetId) {}
};

// 地图单元格
struct MapCell {
    CellType type;
    std::string symbol;
    std::string description;
    std::vector<InteractionType> interactions;
    std::shared_ptr<Item> item;
    
    MapCell(CellType t = CellType::EMPTY, const std::string& sym = ".", 
            const std::string& desc = "")
        : type(t), symbol(sym), description(desc) {}
};

// 地图区块类 - 9x9网格
class MapBlock {
public:
    static const int BLOCK_SIZE = 9;
    
    MapBlock(int id, const std::string& name, MapBlockType type, const std::string& description);
    virtual ~MapBlock() = default;

    // 基本信息
    int getId() const { return id_; }
    std::string getName() const { return name_; }
    MapBlockType getType() const { return type_; }
    std::string getDescription() const { return description_; }
    BlockState getState() const { return state_; }
    void setState(BlockState state) { state_ = state; }

    // 网格操作
    MapCell& getCell(int x, int y);
    const MapCell& getCell(int x, int y) const;
    void setCell(int x, int y, const MapCell& cell);
    bool isValidPosition(int x, int y) const;
    
    // 玩家位置
    void setPlayerPosition(int x, int y);
    std::pair<int, int> getPlayerPosition() const { return {playerX_, playerY_}; }
    
    // 移动检查
    bool canMoveTo(int x, int y) const;
    bool movePlayer(int deltaX, int deltaY);
    
    // 出口检查
    bool isExit(int x, int y) const;
    int getExitTarget(int x, int y) const;
    
    // 交互系统
    virtual InteractionResult interact(Player& player, InteractionType interactionType);
    virtual std::vector<InteractionType> getAvailableInteractions(int x, int y) const;
    
    // 渲染
    std::vector<std::string> render() const;
    std::string getCurrentCellInfo() const;

protected:
    int id_;
    std::string name_;
    MapBlockType type_;
    std::string description_;
    BlockState state_;
    
    // 9x9网格
    MapCell grid_[BLOCK_SIZE][BLOCK_SIZE];
    
    // 玩家位置
    int playerX_, playerY_;
    
    // 出口映射 (x,y) -> targetBlockId
    std::map<std::pair<int, int>, int> exits_;
    
    // 交互处理函数
    std::map<InteractionType, std::function<InteractionResult(Player&, int, int)>> interactionHandlers_;
    
    // 初始化方法
    virtual void initializeGrid() = 0;
    virtual void initializeInteractionHandlers() = 0;
    virtual void initializeExits() = 0;
};

// 教学区块
class TutorialBlock : public MapBlock {
public:
    TutorialBlock();
    
protected:
    void initializeGrid() override;
    void initializeInteractionHandlers() override;
    void initializeExits() override;
    
private:
    InteractionResult handlePickup(Player& player, int x, int y);
    InteractionResult handleMovement(Player& player, int x, int y);
};

// 七天神像区块
class StatueOfSevenBlock : public MapBlock {
public:
    StatueOfSevenBlock();
    
protected:
    void initializeGrid() override;
    void initializeInteractionHandlers() override;
    void initializeExits() override;
    
private:
    InteractionResult handleActivate(Player& player, int x, int y);
    InteractionResult handleChest(Player& player, int x, int y);
    
    bool activated_;
    bool chestOpened_;
};

// 史莱姆战斗区块
class SlimeBattleBlock : public MapBlock {
public:
    SlimeBattleBlock();
    
protected:
    void initializeGrid() override;
    void initializeInteractionHandlers() override;
    void initializeExits() override;
    
private:
    InteractionResult handleBattle(Player& player, int x, int y);
    
    bool battleCompleted_;
    int slimeHealth_;
    int slimeAttack_;
};

// 安伯对话区块
class AmberDialogueBlock : public MapBlock {
public:
    AmberDialogueBlock();
    
protected:
    void initializeGrid() override;
    void initializeInteractionHandlers() override;
    void initializeExits() override;
    
private:
    InteractionResult handleDialogue(Player& player, int x, int y);
    
    bool dialogueCompleted_;
    bool amberJoined_;
};

// 蒙德城区块
class MondstadtCityBlock : public MapBlock {
public:
    MondstadtCityBlock();
    
protected:
    void initializeGrid() override;
    void initializeInteractionHandlers() override;
    void initializeExits() override;
    
private:
    InteractionResult handleEnter(Player& player, int x, int y);
    InteractionResult handleKaeyaDialogue(Player& player, int x, int y);
    
    bool kaeyaJoined_;
};

// 地图管理器V2
class MapManagerV2 {
public:
    MapManagerV2();
    ~MapManagerV2() = default;

    // 地图管理
    void initializeMap();
    void addBlock(std::shared_ptr<MapBlock> block);
    std::shared_ptr<MapBlock> getBlock(int blockId) const;
    std::shared_ptr<MapBlock> getCurrentBlock() const;
    
    // 区块切换
    bool switchToBlock(int blockId, int startX = 4, int startY = 4);
    int getCurrentBlockId() const { return currentBlockId_; }
    
    // 玩家移动
    bool movePlayer(int deltaX, int deltaY);
    std::pair<int, int> getPlayerPosition() const;
    
    // 交互系统
    InteractionResult interactWithCurrentCell(Player& player, InteractionType interactionType);
    std::vector<InteractionType> getAvailableInteractions() const;
    
    // 地图渲染
    std::vector<std::string> renderCurrentBlock() const;
    std::string getCurrentCellInfo() const;
    std::string getBlockInfo() const;
    std::vector<std::string> renderFullMap() const;
    // 拼接区块渲染：以当前区块为中心，按出口方向拼接相邻区块
    // radius 表示拼接的“环数”，1 表示当前区块及上下左右相邻的区块
    std::vector<std::string> renderStitchedBlocks(int radius = 1) const;
    
    // 进度管理
    int getCompletedBlocksCount() const;
    int getTotalBlocksCount() const { return blocks_.size(); }
    bool isMapCompleted() const;

private:
    std::map<int, std::shared_ptr<MapBlock>> blocks_;
    int currentBlockId_;
    
    void handleBlockTransition(int targetBlockId);
};
