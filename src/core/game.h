// =============================================
// 文件: game.h
// 描述: 游戏核心类与状态的声明，负责玩家、地图与存档的协调管理。
// 说明: 仅包含接口与轻量实现，避免引入重依赖；实现见 game.cpp。
// =============================================
#pragma once
#include "../player/player.h"
#include "../storage/storage.h"
#include "map_v2.h"
#include <string>
#include <vector>

// 游戏状态枚举：用于 UI 与逻辑流转判断
enum class GameState {
    MAIN_MENU,
    PLAYING,
    PAUSED,
    INVENTORY,
    SETTINGS
};

// 游戏类 - 管理游戏状态、玩家数据、地图交互与存档
class Game {
public:
    Game();
    ~Game() = default;

    // 游戏状态管理 -----------------------------------------------------------
    // 开始新游戏：初始化玩家、队伍、背包与地图，并进入 PLAYING 状态
    void StartNewGame();
    // 加载游戏：从默认或用户选择的存档恢复（重载版本）
    void LoadGame();
    void SaveGame();
    void LoadGame(const std::string& saveFileName);
    void SaveGame(const std::string& saveFileName);
    // 初始化新玩家（供 StartNewGame 内部调用）
    void InitializeNewPlayer();
    
    // 游戏状态查询 -----------------------------------------------------------
    GameState getCurrentState() const { return currentState_; }
    void setCurrentState(GameState state) { currentState_ = state; }
    
    // 玩家访问 ---------------------------------------------------------------
    Player& getPlayer() { return player_; }
    const Player& getPlayer() const { return player_; }
    
    // 存档管理 ---------------------------------------------------------------
    std::vector<std::string> getSaveFiles() const;
    GameSave::SaveInfo getSaveInfo(const std::string& saveFileName) const;
    bool saveExists(const std::string& saveFileName) const;
    bool deleteSave(const std::string& saveFileName) const;
    
    // 带地图状态的保存和加载 -------------------------------------------------
    void SaveGameWithMapState();
    void LoadGameWithMapState();
    
    // 游戏逻辑 ---------------------------------------------------------------
    // 更新玩家坐标（会直接写入玩家对象；地图移动请优先使用 movePlayer）
    void updatePlayerPosition(int x, int y);
    // 增加经验并自动检测升级
    void addExperience(int exp);
    // 手动触发升级逻辑（重置经验、提升队伍成员等级）
    void levelUp();
    
    // 地图系统 ---------------------------------------------------------------
    MapManagerV2& getMapManager() { return mapManager_; }
    const MapManagerV2& getMapManager() const { return mapManager_; }
    
    // 地图交互 ---------------------------------------------------------------
    InteractionResult interactWithMap(InteractionType interactionType);
    bool movePlayer(int deltaX, int deltaY);
    std::vector<InteractionType> getAvailableMapInteractions() const;

private:
    // 玩家数据（含背包、队伍、经验等级等）
    Player player_;
    // 存档系统
    GameSave gameSave_;
    // 当前游戏状态
    GameState currentState_;
    // 地图管理器（V2 版本：支持区块切换与交互）
    MapManagerV2 mapManager_;
    
    // 辅助方法 ---------------------------------------------------------------
    // 初始化背包基础物品（可根据设计需要扩展）
    void setupInitialInventory();
    // 初始化队伍并装备基础武器
    void setupInitialTeam();
};
