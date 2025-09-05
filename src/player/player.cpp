#include "player.h"
#include "../core/item.h"

Player::Player(std::string name, int startX, int startY)
    : name(name), x(startX), y(startY), level(1), experience(0),
      activeMember(nullptr), inventory(50) {
    // 创建默认队伍成员
    addTeamMember("旅行者", 1);
    setActiveMember(0);
}

InventoryResult Player::addItemToInventory(std::shared_ptr<Item> item) {
    return inventory.addItem(item);
}

InventoryResult Player::removeItemFromInventory(const std::string& itemName, int quantity) {
    return inventory.removeItem(itemName, quantity);
}

void Player::addTeamMember(const std::string& name, int level) {
    auto member = std::make_shared<TeamMember>(name, level);
    teamMembers.push_back(member);
}

void Player::setActiveMember(int index) {
    if (index >= 0 && index < static_cast<int>(teamMembers.size())) {
        activeMember = teamMembers[index];
    }
}

InventoryResult Player::useItem(const std::string& itemName) {
    auto item = inventory.getItem(itemName);
    if (!item) {
        return InventoryResult::NOT_FOUND;
    }

    // 根据物品类型执行不同效果
    switch (item->getType()) {
        case ItemType::FOOD: {
            auto food = std::dynamic_pointer_cast<Food>(item);
            if (food && activeMember) {
                switch (food->getFoodType()) {
                    case FoodType::RECOVERY:
                        activeMember->heal(food->getEffectValue());
                        break;
                    case FoodType::ATTACK:
                        // 暂时增加当前成员的攻击力（后续可以改为临时buff）
                        break;
                    case FoodType::DEFENSE:
                        // 暂时增加当前成员的防御力（后续可以改为临时buff）
                        break;
                    case FoodType::ADVENTURE:
                        // 冒险类效果可以增加经验或其他冒险属性
                        experience += food->getEffectValue();
                        break;
                }
            }
            break;
        }
        case ItemType::WEAPON:
        case ItemType::ARTIFACT:
            // 装备类物品需要单独装备
            return InventoryResult::INVALID_OPERATION;
        case ItemType::MATERIAL:
            // 材料类物品通常不能直接使用
            return InventoryResult::INVALID_OPERATION;
    }

    return inventory.useItem(itemName);
}

bool Player::equipWeaponForMember(int memberIndex, const std::string& weaponName) {
    if (memberIndex < 0 || memberIndex >= static_cast<int>(teamMembers.size())) {
        return false;
    }

    auto item = inventory.getItem(weaponName);
    if (!item || item->getType() != ItemType::WEAPON) {
        return false;
    }

    auto weapon = std::dynamic_pointer_cast<Weapon>(item);
    if (!weapon) {
        return false;
    }

    // 如果该成员已经装备了武器，先卸下
    auto member = teamMembers[memberIndex];
    if (member->getEquippedWeapon()) {
        unequipWeaponFromMember(memberIndex);
    }

    // 为成员装备武器
    if (member->equipWeapon(weapon)) {
        inventory.removeItem(weaponName, 1);
        return true;
    }

    return false;
}

bool Player::equipArtifactForMember(int memberIndex, const std::string& artifactName) {
    if (memberIndex < 0 || memberIndex >= static_cast<int>(teamMembers.size())) {
        return false;
    }

    auto item = inventory.getItem(artifactName);
    if (!item || item->getType() != ItemType::ARTIFACT) {
        return false;
    }

    auto artifact = std::dynamic_pointer_cast<Artifact>(item);
    if (!artifact) {
        return false;
    }

    // 如果该成员已经装备了圣遗物，先卸下
    auto member = teamMembers[memberIndex];
    if (member->getEquippedArtifact()) {
        unequipArtifactFromMember(memberIndex);
    }

    // 为成员装备圣遗物
    if (member->equipArtifact(artifact)) {
        inventory.removeItem(artifactName, 1);
        return true;
    }

    return false;
}

void Player::unequipWeaponFromMember(int memberIndex) {
    if (memberIndex < 0 || memberIndex >= static_cast<int>(teamMembers.size())) {
        return;
    }

    auto member = teamMembers[memberIndex];
    auto weapon = member->getEquippedWeapon();
    if (weapon) {
        // 将武器放回背包
        inventory.addItem(weapon);
        member->unequipWeapon();
    }
}

void Player::unequipArtifactFromMember(int memberIndex) {
    if (memberIndex < 0 || memberIndex >= static_cast<int>(teamMembers.size())) {
        return;
    }

    auto member = teamMembers[memberIndex];
    auto artifact = member->getEquippedArtifact();
    if (artifact) {
        // 将圣遗物放回背包
        inventory.addItem(artifact);
        member->unequipArtifact();
    }
}

int Player::getTotalAttackPower() const {
    return activeMember ? activeMember->getTotalAttack() : 0;
}

int Player::getTotalDefensePower() const {
    return activeMember ? activeMember->getTotalDefense() : 0;
}

void Player::takeDamage(int damage) {
    if (activeMember) {
        activeMember->takeDamage(damage);
    }
}

void Player::heal(int amount) {
    if (activeMember) {
        activeMember->heal(amount);
    }
}

bool Player::isAlive() const {
    return activeMember && activeMember->isAlive();
}