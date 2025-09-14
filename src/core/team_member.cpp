#include "team_member.h"

TeamMember::TeamMember(const std::string& name, int level)
    : name_(name), level_(level), currentHealth_(0), status_(MemberStatus::STANDBY),
      baseHealth_(100), baseAttack_(10), baseDefense_(5),
      equippedWeapon_(nullptr), equippedArtifact_(nullptr) {
    // 根据等级调整基础属性
    baseHealth_ += (level - 1) * 20;
    baseAttack_ += (level - 1) * 5;
    baseDefense_ += (level - 1) * 2;

    currentHealth_ = getTotalHealth();
}

bool TeamMember::equipWeapon(std::shared_ptr<Weapon> weapon) {
    if (!weapon) {
        return false;
    }

    equippedWeapon_ = weapon;
    return true;
}

bool TeamMember::equipArtifact(std::shared_ptr<Artifact> artifact) {
    if (!artifact) {
        return false;
    }

    equippedArtifact_ = artifact;
    return true;
}

void TeamMember::unequipWeapon() {
    equippedWeapon_ = nullptr;
}

void TeamMember::unequipArtifact() {
    equippedArtifact_ = nullptr;
}

int TeamMember::getTotalHealth() const {
    int total = baseHealth_;
    if (equippedArtifact_ && equippedArtifact_->getArtifactType() == ArtifactType::FLOWER_OF_LIFE) {
        // 解析主属性中的数值
        std::string mainStat = equippedArtifact_->getMainStat();
        size_t plusPos = mainStat.find('+');
        if (plusPos != std::string::npos) {
            try {
                int bonus = std::stoi(mainStat.substr(plusPos + 1));
                total += bonus;
            } catch (...) {
                // 解析失败，使用默认值
                total += 100;
            }
        }
    }
    return total;
}

int TeamMember::getTotalAttack() const {
    int total = baseAttack_;
    if (equippedWeapon_) {
        total += equippedWeapon_->getAttackPower();
    }
    if (equippedArtifact_ && equippedArtifact_->getArtifactType() == ArtifactType::PLUME_OF_DEATH) {
        // 解析主属性中的数值
        std::string mainStat = equippedArtifact_->getMainStat();
        size_t plusPos = mainStat.find('+');
        if (plusPos != std::string::npos) {
            try {
                int bonus = std::stoi(mainStat.substr(plusPos + 1));
                total += bonus;
            } catch (...) {
                // 解析失败，使用默认值
                total += 10;
            }
        }
    }
    return total;
}

int TeamMember::getTotalDefense() const {
    int total = baseDefense_;
    // 目前没有装备提供防御加成
    return total;
}

void TeamMember::setCurrentHealth(int health) {
    currentHealth_ = std::max(0, std::min(health, getTotalHealth()));
}

void TeamMember::takeDamage(int damage) {
    int actualDamage = std::max(1, damage - getTotalDefense());
    currentHealth_ -= actualDamage;
    if (currentHealth_ < 0) {
        currentHealth_ = 0;
    }
}

void TeamMember::heal(int amount) {
    currentHealth_ = std::min(getTotalHealth(), currentHealth_ + amount);
}
