#include "storage.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <ctime>
#include <iomanip>
#include <sstream>

GameSave::GameSave() {
    // 确保saves目录存在
    std::filesystem::create_directories("saves");
}

SaveResult GameSave::saveGame(const Player& player, const std::string& saveFileName) {
    return saveGame(player, 0, saveFileName); // 默认区块ID为0
}

SaveResult GameSave::saveGame(const Player& player, int currentBlockId, const std::string& saveFileName) {
    try {
        nlohmann::json saveData;
        
        // 序列化玩家数据
        saveData["player"] = serializePlayer(player);
        
        // 保存地图状态
        saveData["currentBlockId"] = currentBlockId;
        
        // 添加保存时间戳
        saveData["saveTime"] = getCurrentTimeString();
        saveData["version"] = "1.0";
        
        // 写入文件
        std::string filePath = getSaveFilePath(saveFileName);
        std::ofstream file(filePath);
        if (!file.is_open()) {
            return SaveResult::FILE_ERROR;
        }
        
        file << saveData.dump(4); // 格式化输出，缩进4个空格
        file.close();
        
        return SaveResult::SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << "保存游戏时发生错误: " << e.what() << std::endl;
        return SaveResult::SERIALIZATION_ERROR;
    }
}

SaveResult GameSave::loadGame(Player& player, const std::string& saveFileName) {
    int currentBlockId = 0; // 默认值
    return loadGame(player, currentBlockId, saveFileName);
}

SaveResult GameSave::loadGame(Player& player, int& currentBlockId, const std::string& saveFileName) {
    try {
        std::string filePath = getSaveFilePath(saveFileName);
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return SaveResult::FILE_NOT_FOUND;
        }
        
        nlohmann::json saveData;
        file >> saveData;
        file.close();
        
        // 验证数据完整性
        if (!saveData.contains("player")) {
            return SaveResult::INVALID_DATA;
        }
        
        // 反序列化玩家数据
        deserializePlayer(player, saveData["player"]);
        
        // 加载地图状态
        currentBlockId = saveData.value("currentBlockId", 0);
        
        return SaveResult::SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << "加载游戏时发生错误: " << e.what() << std::endl;
        return SaveResult::SERIALIZATION_ERROR;
    }
}

bool GameSave::saveExists(const std::string& saveFileName) const {
    std::string filePath = getSaveFilePath(saveFileName);
    return std::filesystem::exists(filePath);
}

GameSave::SaveInfo GameSave::getSaveInfo(const std::string& saveFileName) const {
    SaveInfo info;
    
    try {
        std::string filePath = getSaveFilePath(saveFileName);
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return info;
        }
        
        nlohmann::json saveData;
        file >> saveData;
        file.close();
        
        if (saveData.contains("player")) {
            const auto& playerData = saveData["player"];
            info.playerName = playerData.value("name", "");
            
            // 安全地获取数值，添加类型检查
            if (playerData.contains("level") && playerData["level"].is_number()) {
                info.level = playerData["level"].get<int>();
            } else {
                info.level = 1;
            }
            
            if (playerData.contains("x") && playerData["x"].is_number()) {
                info.x = playerData["x"].get<int>();
            } else {
                info.x = 0;
            }
            
            if (playerData.contains("y") && playerData["y"].is_number()) {
                info.y = playerData["y"].get<int>();
            } else {
                info.y = 0;
            }
            
            if (playerData.contains("teamSize") && playerData["teamSize"].is_number()) {
                info.teamSize = playerData["teamSize"].get<int>();
            } else {
                info.teamSize = 0;
            }
            
            if (playerData.contains("inventorySize") && playerData["inventorySize"].is_number()) {
                info.inventorySize = playerData["inventorySize"].get<int>();
            } else {
                info.inventorySize = 0;
            }
        }
        
        info.saveTime = saveData.value("saveTime", "");
    } catch (const std::exception& e) {
        std::cerr << "获取存档信息时发生错误: " << e.what() << std::endl;
    }
    
    return info;
}

std::vector<std::string> GameSave::listSaveFiles() const {
    std::vector<std::string> saveFiles;
    
    try {
        std::string savesDir = "saves";
        if (std::filesystem::exists(savesDir)) {
            for (const auto& entry : std::filesystem::directory_iterator(savesDir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".json") {
                    saveFiles.push_back(entry.path().filename().string());
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "列出存档文件时发生错误: " << e.what() << std::endl;
    }
    
    return saveFiles;
}

bool GameSave::deleteSave(const std::string& saveFileName) const {
    try {
        std::string filePath = getSaveFilePath(saveFileName);
        return std::filesystem::remove(filePath);
    } catch (const std::exception& e) {
        std::cerr << "删除存档时发生错误: " << e.what() << std::endl;
        return false;
    }
}

// 序列化方法实现
nlohmann::json GameSave::serializePlayer(const Player& player) const {
    nlohmann::json playerJson;
    
    // 基本信息
    playerJson["name"] = player.name;
    playerJson["x"] = player.x;
    playerJson["y"] = player.y;
    playerJson["level"] = player.level;
    playerJson["experience"] = player.experience;
    
    // 队伍成员
    nlohmann::json teamArray = nlohmann::json::array();
    for (const auto& member : player.teamMembers) {
        teamArray.push_back(serializeTeamMember(*member));
    }
    playerJson["teamMembers"] = teamArray;
    playerJson["teamSize"] = static_cast<int>(player.teamMembers.size());
    
    // 当前活跃成员索引
    int activeIndex = -1;
    for (size_t i = 0; i < player.teamMembers.size(); ++i) {
        if (player.teamMembers[i] == player.activeMember) {
            activeIndex = static_cast<int>(i);
            break;
        }
    }
    playerJson["activeMemberIndex"] = activeIndex;
    
    // 背包
    playerJson["inventory"] = serializeInventory(player.inventory);
    playerJson["inventorySize"] = static_cast<int>(player.inventory.getCurrentSize());
    
    return playerJson;
}

nlohmann::json GameSave::serializeTeamMember(const TeamMember& member) const {
    nlohmann::json memberJson;
    
    memberJson["name"] = member.getName();
    memberJson["level"] = member.getLevel();
    memberJson["currentHealth"] = member.getCurrentHealth();
    memberJson["baseHealth"] = member.getBaseHealth();
    memberJson["baseAttack"] = member.getBaseAttack();
    memberJson["baseDefense"] = member.getBaseDefense();
    // 保存队伍状态
    memberJson["status"] = memberStatusToString(member.getStatus());
    
    // 装备的武器
    if (member.getEquippedWeapon()) {
        memberJson["equippedWeapon"] = serializeItem(*member.getEquippedWeapon());
    }
    
    // 装备的圣遗物
    if (member.getEquippedArtifact()) {
        memberJson["equippedArtifact"] = serializeItem(*member.getEquippedArtifact());
    }
    
    return memberJson;
}

nlohmann::json GameSave::serializeItem(const Item& item) const {
    nlohmann::json itemJson;
    
    itemJson["name"] = item.getName();
    itemJson["type"] = itemTypeToString(item.getType());
    itemJson["rarity"] = rarityToString(item.getRarity());
    itemJson["description"] = item.getDescription();
    itemJson["quantity"] = item.getQuantity();
    
    // 根据物品类型添加特定属性
    switch (item.getType()) {
        case ItemType::WEAPON: {
            const Weapon* weapon = dynamic_cast<const Weapon*>(&item);
            if (weapon) {
                itemJson["weaponType"] = weaponTypeToString(weapon->getWeaponType());
                itemJson["attackPower"] = weapon->getAttackPower();
                itemJson["durability"] = weapon->getDurability();
            }
            break;
        }
        case ItemType::ARTIFACT: {
            const Artifact* artifact = dynamic_cast<const Artifact*>(&item);
            if (artifact) {
                itemJson["artifactType"] = artifactTypeToString(artifact->getArtifactType());
                itemJson["mainStat"] = artifact->getMainStat();
                itemJson["subStats"] = artifact->getSubStats();
            }
            break;
        }
        case ItemType::FOOD: {
            const Food* food = dynamic_cast<const Food*>(&item);
            if (food) {
                itemJson["foodType"] = foodTypeToString(food->getFoodType());
                itemJson["effectValue"] = food->getEffectValue();
                itemJson["duration"] = food->getDuration();
            }
            break;
        }
        case ItemType::MATERIAL: {
            const Material* material = dynamic_cast<const Material*>(&item);
            if (material) {
                itemJson["materialType"] = materialTypeToString(material->getMaterialType());
                itemJson["isStackable"] = material->isStackable();
            }
            break;
        }
    }
    
    return itemJson;
}

nlohmann::json GameSave::serializeInventory(const Inventory& inventory) const {
    nlohmann::json inventoryJson;
    
    inventoryJson["maxCapacity"] = inventory.getMaxCapacity();
    
    nlohmann::json itemsArray = nlohmann::json::array();
    auto allItems = inventory.getAllItems();
    for (const auto& item : allItems) {
        itemsArray.push_back(serializeItem(*item));
    }
    inventoryJson["items"] = itemsArray;
    
    return inventoryJson;
}

// 反序列化方法实现
void GameSave::deserializePlayer(Player& player, const nlohmann::json& json) const {
    // 基本信息
    player.name = json.value("name", "");
    player.x = json.value("x", 0);
    player.y = json.value("y", 0);
    player.level = json.value("level", 1);
    player.experience = json.value("experience", 0);
    
    // 清空现有队伍成员
    player.teamMembers.clear();
    player.activeMember = nullptr;
    
    // 加载队伍成员
    if (json.contains("teamMembers") && json["teamMembers"].is_array()) {
        for (const auto& memberJson : json["teamMembers"]) {
            auto member = deserializeTeamMember(memberJson);
            if (member) {
                player.teamMembers.push_back(member);
            }
        }
    }
    
    // 设置活跃成员
    int activeIndex = json.value("activeMemberIndex", -1);
    if (activeIndex >= 0 && activeIndex < static_cast<int>(player.teamMembers.size())) {
        player.activeMember = player.teamMembers[activeIndex];
        // 确保该成员标记为上场
        if (player.activeMember && !player.activeMember->isActive()) {
            player.activeMember->setStatus(MemberStatus::ACTIVE);
        }
    } else if (!player.teamMembers.empty()) {
        player.activeMember = player.teamMembers[0];
        if (player.activeMember && !player.activeMember->isActive()) {
            player.activeMember->setStatus(MemberStatus::ACTIVE);
        }
    }
    
    // 加载背包
    if (json.contains("inventory")) {
        deserializeInventory(player.inventory, json["inventory"]);
    }
}

std::shared_ptr<TeamMember> GameSave::deserializeTeamMember(const nlohmann::json& json) const {
    try {
        std::string name = json.value("name", "");
        int level = json.value("level", 1);
        
        auto member = std::make_shared<TeamMember>(name, level);
        
        // 设置当前生命值
        member->setCurrentHealth(json.value("currentHealth", member->getTotalHealth()));
        
        // 加载装备的武器
        if (json.contains("equippedWeapon")) {
            auto weapon = deserializeItem(json["equippedWeapon"]);
            if (weapon && weapon->getType() == ItemType::WEAPON) {
                auto weaponPtr = std::dynamic_pointer_cast<Weapon>(weapon);
                if (weaponPtr) {
                    member->equipWeapon(weaponPtr);
                }
            }
        }
        
        // 加载装备的圣遗物
        if (json.contains("equippedArtifact")) {
            auto artifact = deserializeItem(json["equippedArtifact"]);
            if (artifact && artifact->getType() == ItemType::ARTIFACT) {
                auto artifactPtr = std::dynamic_pointer_cast<Artifact>(artifact);
                if (artifactPtr) {
                    member->equipArtifact(artifactPtr);
                }
            }
        }
        
        // 加载队伍状态（默认为待命）
        if (json.contains("status")) {
            std::string statusStr = json.value("status", "STANDBY");
            member->setStatus(stringToMemberStatus(statusStr));
        }
        
        return member;
    } catch (const std::exception& e) {
        std::cerr << "反序列化队伍成员时发生错误: " << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<Item> GameSave::deserializeItem(const nlohmann::json& json) const {
    try {
        std::string name = json.value("name", "");
        ItemType type = stringToItemType(json.value("type", ""));
        Rarity rarity = stringToRarity(json.value("rarity", "ONE_STAR"));
        std::string description = json.value("description", "");
        int quantity = json.value("quantity", 1);
        
        std::shared_ptr<Item> item;
        
        switch (type) {
            case ItemType::WEAPON: {
                WeaponType weaponType = stringToWeaponType(json.value("weaponType", ""));
                int attackPower = json.value("attackPower", 0);
                int durability = json.value("durability", 100);
                
                item = std::make_shared<Weapon>(name, weaponType, rarity, description, attackPower, durability);
                break;
            }
            case ItemType::ARTIFACT: {
                ArtifactType artifactType = stringToArtifactType(json.value("artifactType", ""));
                std::string mainStat = json.value("mainStat", "");
                std::vector<std::string> subStats = json.value("subStats", std::vector<std::string>());
                
                item = std::make_shared<Artifact>(name, artifactType, rarity, description, mainStat, subStats);
                break;
            }
            case ItemType::FOOD: {
                FoodType foodType = stringToFoodType(json.value("foodType", ""));
                int effectValue = json.value("effectValue", 0);
                int duration = json.value("duration", 0);
                
                item = std::make_shared<Food>(name, foodType, rarity, description, effectValue, duration);
                break;
            }
            case ItemType::MATERIAL: {
                MaterialType materialType = stringToMaterialType(json.value("materialType", ""));
                bool isStackable = json.value("isStackable", true);
                
                item = std::make_shared<Material>(name, materialType, rarity, description, isStackable);
                break;
            }
            default:
                return nullptr;
        }
        
        if (item) {
            item->setQuantity(quantity);
        }
        
        return item;
    } catch (const std::exception& e) {
        std::cerr << "反序列化物品时发生错误: " << e.what() << std::endl;
        return nullptr;
    }
}

void GameSave::deserializeInventory(Inventory& inventory, const nlohmann::json& json) const {
    try {
        // 设置背包容量
        size_t maxCapacity = json.value("maxCapacity", 100);
        inventory.setMaxCapacity(maxCapacity);
        
        // 清空现有物品
        inventory.removeAllItems();
        
        // 加载物品
        if (json.contains("items") && json["items"].is_array()) {
            for (const auto& itemJson : json["items"]) {
                auto item = deserializeItem(itemJson);
                if (item) {
                    inventory.addItem(item);
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "反序列化背包时发生错误: " << e.what() << std::endl;
    }
}

// 辅助方法实现
std::string GameSave::getCurrentTimeString() const {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string GameSave::getSaveFilePath(const std::string& fileName) const {
    return "saves/" + fileName;
}

// 类型转换方法实现
std::string GameSave::itemTypeToString(ItemType type) const {
    switch (type) {
        case ItemType::WEAPON: return "WEAPON";
        case ItemType::ARTIFACT: return "ARTIFACT";
        case ItemType::FOOD: return "FOOD";
        case ItemType::MATERIAL: return "MATERIAL";
        default: return "UNKNOWN";
    }
}

ItemType GameSave::stringToItemType(const std::string& str) const {
    if (str == "WEAPON") return ItemType::WEAPON;
    if (str == "ARTIFACT") return ItemType::ARTIFACT;
    if (str == "FOOD") return ItemType::FOOD;
    if (str == "MATERIAL") return ItemType::MATERIAL;
    return ItemType::WEAPON; // 默认值
}

std::string GameSave::weaponTypeToString(WeaponType type) const {
    switch (type) {
        case WeaponType::ONE_HANDED_SWORD: return "ONE_HANDED_SWORD";
        case WeaponType::TWO_HANDED_SWORD: return "TWO_HANDED_SWORD";
        case WeaponType::BOW: return "BOW";
        case WeaponType::CATALYST: return "CATALYST";
        default: return "ONE_HANDED_SWORD";
    }
}

WeaponType GameSave::stringToWeaponType(const std::string& str) const {
    if (str == "ONE_HANDED_SWORD") return WeaponType::ONE_HANDED_SWORD;
    if (str == "TWO_HANDED_SWORD") return WeaponType::TWO_HANDED_SWORD;
    if (str == "BOW") return WeaponType::BOW;
    if (str == "CATALYST") return WeaponType::CATALYST;
    return WeaponType::ONE_HANDED_SWORD;
}

std::string GameSave::artifactTypeToString(ArtifactType type) const {
    switch (type) {
        case ArtifactType::FLOWER_OF_LIFE: return "FLOWER_OF_LIFE";
        case ArtifactType::PLUME_OF_DEATH: return "PLUME_OF_DEATH";
        case ArtifactType::SANDS_OF_EON: return "SANDS_OF_EON";
        case ArtifactType::GOBLET_OF_EONOTHEM: return "GOBLET_OF_EONOTHEM";
        case ArtifactType::CIRCLET_OF_LOGOS: return "CIRCLET_OF_LOGOS";
        default: return "FLOWER_OF_LIFE";
    }
}

ArtifactType GameSave::stringToArtifactType(const std::string& str) const {
    if (str == "FLOWER_OF_LIFE") return ArtifactType::FLOWER_OF_LIFE;
    if (str == "PLUME_OF_DEATH") return ArtifactType::PLUME_OF_DEATH;
    if (str == "SANDS_OF_EON") return ArtifactType::SANDS_OF_EON;
    if (str == "GOBLET_OF_EONOTHEM") return ArtifactType::GOBLET_OF_EONOTHEM;
    if (str == "CIRCLET_OF_LOGOS") return ArtifactType::CIRCLET_OF_LOGOS;
    return ArtifactType::FLOWER_OF_LIFE;
}

std::string GameSave::foodTypeToString(FoodType type) const {
    switch (type) {
        case FoodType::RECOVERY: return "RECOVERY";
        case FoodType::ATTACK: return "ATTACK";
        case FoodType::ADVENTURE: return "ADVENTURE";
        case FoodType::DEFENSE: return "DEFENSE";
        default: return "RECOVERY";
    }
}

FoodType GameSave::stringToFoodType(const std::string& str) const {
    if (str == "RECOVERY") return FoodType::RECOVERY;
    if (str == "ATTACK") return FoodType::ATTACK;
    if (str == "ADVENTURE") return FoodType::ADVENTURE;
    if (str == "DEFENSE") return FoodType::DEFENSE;
    return FoodType::RECOVERY;
}

std::string GameSave::materialTypeToString(MaterialType type) const {
    switch (type) {
        case MaterialType::MONSTER_DROP: return "MONSTER_DROP";
        case MaterialType::COOKING_INGREDIENT: return "COOKING_INGREDIENT";
        default: return "MONSTER_DROP";
    }
}

MaterialType GameSave::stringToMaterialType(const std::string& str) const {
    if (str == "MONSTER_DROP") return MaterialType::MONSTER_DROP;
    if (str == "COOKING_INGREDIENT") return MaterialType::COOKING_INGREDIENT;
    return MaterialType::MONSTER_DROP;
}

std::string GameSave::rarityToString(Rarity rarity) const {
    switch (rarity) {
        case Rarity::ONE_STAR: return "ONE_STAR";
        case Rarity::TWO_STAR: return "TWO_STAR";
        case Rarity::THREE_STAR: return "THREE_STAR";
        case Rarity::FOUR_STAR: return "FOUR_STAR";
        case Rarity::FIVE_STAR: return "FIVE_STAR";
        default: return "ONE_STAR";
    }
}

Rarity GameSave::stringToRarity(const std::string& str) const {
    if (str == "ONE_STAR") return Rarity::ONE_STAR;
    if (str == "TWO_STAR") return Rarity::TWO_STAR;
    if (str == "THREE_STAR") return Rarity::THREE_STAR;
    if (str == "FOUR_STAR") return Rarity::FOUR_STAR;
    if (str == "FIVE_STAR") return Rarity::FIVE_STAR;
    return Rarity::ONE_STAR;
}

// 队伍成员状态转换
std::string GameSave::memberStatusToString(MemberStatus status) const {
    switch (status) {
        case MemberStatus::ACTIVE:
            return "ACTIVE";
        case MemberStatus::STANDBY:
            return "STANDBY";
        case MemberStatus::INJURED:
            return "INJURED";
        default:
            return "STANDBY";
    }
}

MemberStatus GameSave::stringToMemberStatus(const std::string& str) const {
    if (str == "ACTIVE") return MemberStatus::ACTIVE;
    if (str == "INJURED") return MemberStatus::INJURED;
    return MemberStatus::STANDBY;
}
