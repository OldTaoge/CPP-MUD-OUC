#pragma once
#include <string>
#include <vector>
#include <memory>
#include "../player/player.h"
#include "../player/item.h"
#include "../player/quest.h"
#include "../storage/storage.h"

// 队伍成员状态
struct TeamMember {
    std::string name;
    int level;
    int health;
    int maxHealth;
    int attack;
    int defense;
    std::string element; // 元素属性
    std::string weapon; // 装备武器
    
    TeamMember(const std::string& name = "", int level = 1, 
               int health = 100, int maxHealth = 100,
               int attack = 10, int defense = 5,
               const std::string& element = "无",
               const std::string& weapon = "无");
    
    std::string getStatusString() const;
};

// 位置信息
struct Position {
    std::string areaName;    // 区域名称
    std::string location;    // 具体位置
    int x;                   // X坐标
    int y;                   // Y坐标
    std::string description; // 位置描述
    
    Position(const std::string& area = "", const std::string& loc = "",
             int x = 0, int y = 0, const std::string& desc = "");
    
    std::string getFullPosition() const;
};

// 游戏状态管理类
class GameState {
public:
    // 单例模式访问
    static GameState& getInstance();
    
    // 禁止拷贝和赋值
    GameState(const GameState&) = delete;
    GameState& operator=(const GameState&) = delete;
    
    // 玩家相关方法
    void setPlayer(std::shared_ptr<Player> player);
    std::shared_ptr<Player> getPlayer() const;
    
    // 队伍管理
    void addTeamMember(const TeamMember& member);
    void removeTeamMember(const std::string& name);
    const std::vector<TeamMember>& getTeam() const;
    TeamMember* findTeamMember(const std::string& name);
    void updateTeamMemberHealth(const std::string& name, int health);
    
    // 背包管理
    void addToInventory(const Item& item);
    bool removeFromInventory(int index);
    const std::vector<Item>& getInventory() const;
    Item* findItemInInventory(const std::string& itemName);
    int getInventorySize() const;
    
    // 位置管理
    void setPosition(const Position& position);
    Position getPosition() const;
    void moveTo(const std::string& area, const std::string& location, 
                int x = 0, int y = 0, const std::string& description = "");
    
    // 任务管理
    void addQuest(const Quest& quest);
    const std::vector<Quest>& getQuests() const;
    Quest* findQuest(const std::string& questId);
    
    // 状态保存和加载
    bool saveGame(const std::string& filename = "savegame.json");
    bool loadGame(const std::string& filename = "savegame.json");
    bool saveGameExists(const std::string& filename = "savegame.json") const;
    
    // 游戏时间
    void updateGameTime(int minutes = 1);
    std::string getGameTimeString() const;
    
private:
    GameState(); // 私有构造函数
    
    std::shared_ptr<Player> player_;          // 主玩家
    std::vector<TeamMember> team_;           // 队伍成员
    std::vector<Item> inventory_;           // 背包物品
    Position currentPosition_;               // 当前位置
    std::vector<Quest> activeQuests_;       // 活跃任务
    
    // 游戏时间（以分钟为单位）
    int gameTimeMinutes_;
};