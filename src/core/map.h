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
    ACTIVATE,   // 激活（如七天神像）
    ENTER       // 进入
};

// 前向声明
class MapBlock;
class MapManager;

// 交互结果结构
struct InteractionResult {
    bool success;
    std::string message;
    std::vector<std::shared_ptr<Item>> rewards;
    bool blockCompleted;
    
    InteractionResult(bool s = false, const std::string& msg = "", 
                     const std::vector<std::shared_ptr<Item>>& r = {},
                     bool completed = false)
        : success(s), message(msg), rewards(r), blockCompleted(completed) {}
};

// 地图区块基类
class MapBlock {
public:
    MapBlock(const std::string& name, MapBlockType type, int x, int y, 
             const std::string& description);
    virtual ~MapBlock() = default;

    // 基本信息
    std::string getName() const { return name_; }
    MapBlockType getType() const { return type_; }
    int getX() const { return x_; }
    int getY() const { return y_; }
    std::string getDescription() const { return description_; }
    BlockState getState() const { return state_; }
    void setState(BlockState state) { state_ = state; }

    // 交互系统
    virtual InteractionResult interact(Player& player, InteractionType interactionType);
    virtual bool canInteract(InteractionType interactionType) const;
    virtual std::vector<InteractionType> getAvailableInteractions() const;

    // 渲染信息
    virtual std::string getDisplaySymbol() const;
    virtual std::string getDisplayName() const;

protected:
    std::string name_;
    MapBlockType type_;
    int x_, y_;
    std::string description_;
    BlockState state_;
    
    // 交互处理函数映射
    std::map<InteractionType, std::function<InteractionResult(Player&)>> interactionHandlers_;
    
    // 初始化交互处理器
    virtual void initializeInteractionHandlers() = 0;
};

// 教学区块 - 教会用户基本操作
class TutorialBlock : public MapBlock {
public:
    TutorialBlock(int x, int y);
    
protected:
    void initializeInteractionHandlers() override;
    InteractionResult handlePickup(Player& player);
    InteractionResult handleMovement(Player& player);
};

// 七天神像区块 - 给予风元素和宝箱奖励
class StatueOfSevenBlock : public MapBlock {
public:
    StatueOfSevenBlock(int x, int y);
    
protected:
    void initializeInteractionHandlers() override;
    InteractionResult handleActivate(Player& player);
    InteractionResult handleChest(Player& player);
    
private:
    bool activated_;
    bool chestOpened_;
};

// 史莱姆战斗区块
class SlimeBattleBlock : public MapBlock {
public:
    SlimeBattleBlock(int x, int y);
    
protected:
    void initializeInteractionHandlers() override;
    InteractionResult handleBattle(Player& player);
    
private:
    bool battleCompleted_;
    int slimeHealth_;
    int slimeAttack_;
};

// 安伯对话区块
class AmberDialogueBlock : public MapBlock {
public:
    AmberDialogueBlock(int x, int y);
    
protected:
    void initializeInteractionHandlers() override;
    InteractionResult handleDialogue(Player& player);
    
private:
    bool dialogueCompleted_;
    bool amberJoined_;
};

// 蒙德城区块
class MondstadtCityBlock : public MapBlock {
public:
    MondstadtCityBlock(int x, int y);
    
protected:
    void initializeInteractionHandlers() override;
    InteractionResult handleEnter(Player& player);
    InteractionResult handleKaeyaDialogue(Player& player);
    
private:
    bool kaeyaJoined_;
};

// 地图管理器
class MapManager {
public:
    MapManager();
    ~MapManager() = default;

    // 地图管理
    void initializeMap();
    void addBlock(std::shared_ptr<MapBlock> block);
    std::shared_ptr<MapBlock> getBlock(int x, int y) const;
    std::vector<std::shared_ptr<MapBlock>> getAllBlocks() const { return blocks_; }
    
    // 玩家位置管理
    void setPlayerPosition(int x, int y);
    std::pair<int, int> getPlayerPosition() const { return {playerX_, playerY_}; }
    
    // 移动系统
    bool canMoveTo(int x, int y) const;
    bool movePlayer(int deltaX, int deltaY);
    
    // 交互系统
    InteractionResult interactWithCurrentBlock(Player& player, InteractionType interactionType);
    std::vector<InteractionType> getAvailableInteractions() const;
    
    // 地图渲染
    std::vector<std::string> renderMap(int width, int height) const;
    std::string getCurrentBlockInfo() const;
    
    // 进度管理
    int getCompletedBlocksCount() const;
    int getTotalBlocksCount() const { return blocks_.size(); }
    bool isMapCompleted() const;

private:
    std::vector<std::shared_ptr<MapBlock>> blocks_;
    std::map<std::pair<int, int>, std::shared_ptr<MapBlock>> blockMap_;
    int playerX_, playerY_;
    
    // 地图边界
    int minX_, maxX_, minY_, maxY_;
    
    void updateMapBounds();
    std::string getBlockSymbolAt(int x, int y) const;
};
