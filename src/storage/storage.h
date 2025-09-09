#pragma once
#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include "../player/player.h"
#include "../core/team_member.h"
#include "../core/inventory.h"
#include "../core/item.h"

// 保存系统结果枚举
enum class SaveResult {
    SUCCESS,
    FILE_ERROR,
    SERIALIZATION_ERROR,
    INVALID_DATA,
    FILE_NOT_FOUND
};

// 游戏状态保存类
class GameSave {
public:
    // 构造函数
    GameSave();
    ~GameSave() = default;

    // 保存游戏状态
    SaveResult saveGame(const Player& player, const std::string& saveFileName = "save.json");
    SaveResult saveGame(const Player& player, int currentBlockId, const std::string& saveFileName = "save.json");
    
    // 加载游戏状态
    SaveResult loadGame(Player& player, const std::string& saveFileName = "save.json");
    SaveResult loadGame(Player& player, int& currentBlockId, const std::string& saveFileName = "save.json");
    
    // 检查存档是否存在
    bool saveExists(const std::string& saveFileName = "save.json") const;
    
    // 获取存档信息
    struct SaveInfo {
        std::string playerName;
        int level;
        int x, y;
        std::string saveTime;
        size_t teamSize;
        size_t inventorySize;
    };
    
    SaveInfo getSaveInfo(const std::string& saveFileName = "save.json") const;
    
    // 列出所有存档文件
    std::vector<std::string> listSaveFiles() const;
    
    // 删除存档
    bool deleteSave(const std::string& saveFileName) const;

private:
    // 序列化相关方法
    nlohmann::json serializePlayer(const Player& player) const;
    nlohmann::json serializeTeamMember(const TeamMember& member) const;
    nlohmann::json serializeItem(const Item& item) const;
    nlohmann::json serializeInventory(const Inventory& inventory) const;
    
    // 反序列化相关方法
    void deserializePlayer(Player& player, const nlohmann::json& json) const;
    std::shared_ptr<TeamMember> deserializeTeamMember(const nlohmann::json& json) const;
    std::shared_ptr<Item> deserializeItem(const nlohmann::json& json) const;
    void deserializeInventory(Inventory& inventory, const nlohmann::json& json) const;
    
    // 辅助方法
    std::string getCurrentTimeString() const;
    std::string getSaveFilePath(const std::string& fileName) const;
    
    // 物品类型转换
    std::string itemTypeToString(ItemType type) const;
    ItemType stringToItemType(const std::string& str) const;
    
    std::string weaponTypeToString(WeaponType type) const;
    WeaponType stringToWeaponType(const std::string& str) const;
    
    std::string artifactTypeToString(ArtifactType type) const;
    ArtifactType stringToArtifactType(const std::string& str) const;
    
    std::string foodTypeToString(FoodType type) const;
    FoodType stringToFoodType(const std::string& str) const;
    
    std::string materialTypeToString(MaterialType type) const;
    MaterialType stringToMaterialType(const std::string& str) const;
    
    std::string rarityToString(Rarity rarity) const;
    Rarity stringToRarity(const std::string& str) const;

    // 队伍成员状态转换
    std::string memberStatusToString(MemberStatus status) const;
    MemberStatus stringToMemberStatus(const std::string& str) const;
};
