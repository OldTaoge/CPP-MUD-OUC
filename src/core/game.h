#pragma once
#include "../player/player.h"
#include "../storage/storage.h"
#include "map_v2.h"
#include <string>
#include <vector>

// 游戏状态枚举
enum class GameState {
    MAIN_MENU,
    PLAYING,
    PAUSED,
    INVENTORY,
    SETTINGS
};

// 游戏类 - 管理游戏状态和逻辑
class Game {
public:
    Game();
    ~Game() = default;

    // 游戏状态管理
    void StartNewGame();
    void LoadGame();
    void SaveGame();
    void InitializeNewPlayer();
    
    // 游戏状态查询
    GameState getCurrentState() const { return currentState_; }
    void setCurrentState(GameState state) { currentState_ = state; }
    
    // 玩家访问
    Player& getPlayer() { return player_; }
    const Player& getPlayer() const { return player_; }
    
    // 存档管理
    std::vector<std::string> getSaveFiles() const;
    GameSave::SaveInfo getSaveInfo(const std::string& saveFileName) const;
    bool saveExists(const std::string& saveFileName) const;
    bool deleteSave(const std::string& saveFileName) const;
    
    // 游戏逻辑
    void updatePlayerPosition(int x, int y);
    void addExperience(int exp);
    void levelUp();
    
    // 地图系统
    MapManagerV2& getMapManager() { return mapManager_; }
    const MapManagerV2& getMapManager() const { return mapManager_; }
    
    // 地图交互
    InteractionResult interactWithMap(InteractionType interactionType);
    bool movePlayer(int deltaX, int deltaY);
    std::vector<InteractionType> getAvailableMapInteractions() const;

private:
    Player player_;
    GameSave gameSave_;
    GameState currentState_;
    MapManagerV2 mapManager_;
    
    // 辅助方法
    void setupInitialInventory();
    void setupInitialTeam();
};
