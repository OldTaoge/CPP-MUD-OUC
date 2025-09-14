#include "gamestate.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include "../vendor/nlohmann/json.hpp"

using json = nlohmann::json;

// TeamMember 实现
TeamMember::TeamMember(const std::string& name, int level, 
                       int health, int maxHealth,
                       int attack, int defense,
                       const std::string& element,
                       const std::string& weapon)
    : name(name), level(level), health(health), maxHealth(maxHealth),
      attack(attack), defense(defense), element(element), weapon(weapon) {}

std::string TeamMember::getStatusString() const {
    return name + " Lv." + std::to_string(level) + 
           " HP: " + std::to_string(health) + "/" + std::to_string(maxHealth) +
           " 元素: " + element + " 武器: " + weapon;
}

// Position 实现
Position::Position(const std::string& area, const std::string& loc,
                   int x, int y, const std::string& desc)
    : areaName(area), location(loc), x(x), y(y), description(desc) {}

std::string Position::getFullPosition() const {
    return areaName + " - " + location + 
           " (" + std::to_string(x) + ", " + std::to_string(y) + ")";
}

// GameState 单例实现
GameState& GameState::getInstance() {
    static GameState instance;
    return instance;
}

GameState::GameState() 
    : gameTimeMinutes_(0) {
    // 初始化默认位置
    currentPosition_ = Position("蒙德城", "广场", 0, 0, "蒙德城的中心广场");
}

// 玩家管理
void GameState::setPlayer(std::shared_ptr<Player> player) {
    player_ = player;
}

std::shared_ptr<Player> GameState::getPlayer() const {
    return player_;
}

// 队伍管理
void GameState::addTeamMember(const TeamMember& member) {
    // 检查是否已存在同名成员
    auto it = std::find_if(team_.begin(), team_.end(),
        [&member](const TeamMember& m) { return m.name == member.name; });
    
    if (it == team_.end()) {
        team_.push_back(member);
    }
}

void GameState::removeTeamMember(const std::string& name) {
    team_.erase(std::remove_if(team_.begin(), team_.end(),
        [&name](const TeamMember& m) { return m.name == name; }), team_.end());
}

const std::vector<TeamMember>& GameState::getTeam() const {
    return team_;
}

TeamMember* GameState::findTeamMember(const std::string& name) {
    auto it = std::find_if(team_.begin(), team_.end(),
        [&name](const TeamMember& m) { return m.name == name; });
    
    return (it != team_.end()) ? &(*it) : nullptr;
}

void GameState::updateTeamMemberHealth(const std::string& name, int health) {
    TeamMember* member = findTeamMember(name);
    if (member) {
        member->health = std::max(0, std::min(health, member->maxHealth));
    }
}

// 背包管理
void GameState::addToInventory(const Item& item) {
    // 如果物品可堆叠，检查是否已存在同名物品
    if (item.isStackable) {
        for (auto& existingItem : inventory_) {
            if (existingItem.name == item.name) {
                existingItem.addStack(item.stackSize);
                return;
            }
        }
    }
    // 如果物品不可堆叠或者物品栏中没有同名物品，则直接添加
    inventory_.push_back(item);
}

bool GameState::removeFromInventory(int index) {
    if (index < 0 || index >= inventory_.size()) {
        return false;
    }
    inventory_.erase(inventory_.begin() + index);
    return true;
}

const std::vector<Item>& GameState::getInventory() const {
    return inventory_;
}

Item* GameState::findItemInInventory(const std::string& itemName) {
    for (auto& item : inventory_) {
        if (item.name == itemName) {
            return &item;
        }
    }
    return nullptr;
}

int GameState::getInventorySize() const {
    return inventory_.size();
}

// 位置管理
void GameState::setPosition(const Position& position) {
    currentPosition_ = position;
}

Position GameState::getPosition() const {
    return currentPosition_;
}

void GameState::moveTo(const std::string& area, const std::string& location, 
                      int x, int y, const std::string& description) {
    currentPosition_ = Position(area, location, x, y, description);
    updateGameTime(30); // 移动消耗30分钟游戏时间
}

// 任务管理
void GameState::addQuest(const Quest& quest) {
    // 检查是否已存在同ID任务
    auto it = std::find_if(activeQuests_.begin(), activeQuests_.end(),
        [&quest](const Quest& q) { return q.id == quest.id; });
    
    if (it == activeQuests_.end()) {
        activeQuests_.push_back(quest);
    }
}

const std::vector<Quest>& GameState::getQuests() const {
    return activeQuests_;
}

Quest* GameState::findQuest(const std::string& questId) {
    auto it = std::find_if(activeQuests_.begin(), activeQuests_.end(),
        [&questId](const Quest& q) { return q.id == questId; });
    
    return (it != activeQuests_.end()) ? &(*it) : nullptr;
}

// 游戏时间管理
void GameState::updateGameTime(int minutes) {
    gameTimeMinutes_ += minutes;
}

std::string GameState::getGameTimeString() const {
    int days = gameTimeMinutes_ / (24 * 60);
    int hours = (gameTimeMinutes_ % (24 * 60)) / 60;
    int minutes = gameTimeMinutes_ % 60;
    
    return "第" + std::to_string(days + 1) + "天 " + 
           std::to_string(hours) + ":" + 
           (minutes < 10 ? "0" : "") + std::to_string(minutes);
}

// 保存和加载游戏
bool GameState::saveGame(const std::string& filename) {
    try {
        json gameData;
        
        // 保存玩家数据
        if (player_) {
            gameData["player"] = {
                {"name", player_->name},
                {"x", player_->x},
                {"y", player_->y},
                {"health", player_->health}
            };
            
            // 保存玩家物品栏
            json inventoryArray;
            for (const auto& item : player_->inventory) {
                inventoryArray.push_back({
                    {"name", item.name},
                    {"stackSize", item.stackSize}
                });
            }
            gameData["player"]["inventory"] = inventoryArray;
        }
        
        // 保存队伍数据
        json teamArray;
        for (const auto& member : team_) {
            teamArray.push_back({
                {"name", member.name},
                {"level", member.level},
                {"health", member.health},
                {"maxHealth", member.maxHealth},
                {"attack", member.attack},
                {"defense", member.defense},
                {"element", member.element},
                {"weapon", member.weapon}
            });
        }
        gameData["team"] = teamArray;
        
        // 保存位置数据
        gameData["position"] = {
            {"area", currentPosition_.areaName},
            {"location", currentPosition_.location},
            {"x", currentPosition_.x},
            {"y", currentPosition_.y},
            {"description", currentPosition_.description}
        };
        
        // 保存游戏时间
        gameData["gameTime"] = gameTimeMinutes_;
        
        // 使用storage模块保存数据
        Storage& storage = Storage::getInstance();
        return storage.saveString(filename, gameData.dump(4));
    } catch (const std::exception& e) {
        std::cerr << "保存游戏失败: " << e.what() << std::endl;
        return false;
    }
}

bool GameState::saveGameExists(const std::string& filename) const {
    Storage& storage = Storage::getInstance();
    return storage.fileExists(filename);
}

bool GameState::loadGame(const std::string& filename) {
    try {
        Storage& storage = Storage::getInstance();
        std::string saveData = storage.loadString(filename);
        
        if (saveData.empty()) {
            return false;
        }
        
        json gameData = json::parse(saveData);
        
        // 加载玩家数据
        if (gameData.contains("player")) {
            auto playerData = gameData["player"];
            if (!player_) {
                player_ = std::make_shared<Player>(
                    playerData["name"].get<std::string>(),
                    playerData["x"].get<int>(),
                    playerData["y"].get<int>(),
                    playerData["health"].get<int>()
                );
            } else {
                player_->name = playerData["name"].get<std::string>();
                player_->x = playerData["x"].get<int>();
                player_->y = playerData["y"].get<int>();
                player_->health = playerData["health"].get<int>();
            }
            
            // 加载玩家物品栏
            if (playerData.contains("inventory")) {
                player_->inventory.clear();
                for (const auto& itemData : playerData["inventory"]) {
                    // 这里需要根据实际物品类型创建物品
                    Item item(itemData["name"].get<std::string>());
                    item.stackSize = itemData["stackSize"].get<int>();
                    player_->inventory.push_back(item);
                }
            }
        }
        
        // 加载队伍数据
        if (gameData.contains("team")) {
            team_.clear();
            for (const auto& memberData : gameData["team"]) {
                TeamMember member(
                    memberData["name"].get<std::string>(),
                    memberData["level"].get<int>(),
                    memberData["health"].get<int>(),
                    memberData["maxHealth"].get<int>(),
                    memberData["attack"].get<int>(),
                    memberData["defense"].get<int>(),
                    memberData["element"].get<std::string>(),
                    memberData["weapon"].get<std::string>()
                );
                team_.push_back(member);
            }
        }
        
        // 加载位置数据
        if (gameData.contains("position")) {
            auto posData = gameData["position"];
            currentPosition_ = Position(
                posData["area"].get<std::string>(),
                posData["location"].get<std::string>(),
                posData["x"].get<int>(),
                posData["y"].get<int>(),
                posData["description"].get<std::string>()
            );
        }
        
        // 加载游戏时间
        if (gameData.contains("gameTime")) {
            gameTimeMinutes_ = gameData["gameTime"].get<int>();
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "加载游戏失败: " << e.what() << std::endl;
        return false;
    }
}