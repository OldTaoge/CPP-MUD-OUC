// =============================================
// 文件: inventory.cpp
// 描述: 背包系统实现。提供物品管理、排序搜索与事件通知。
// =============================================
#include "inventory.h"
#include <algorithm>
#include <sstream>

// 背包类实现
Inventory::Inventory(size_t maxCapacity)
    : maxCapacity_(maxCapacity), itemChangeCallback_(nullptr) {
}

InventoryResult Inventory::addItem(std::shared_ptr<Item> item) {
    if (!item) {
        return InventoryResult::INVALID_OPERATION;
    }

    // 检查背包是否已满
    if (isFull()) {
        return InventoryResult::FULL;
    }

    // 对于可堆叠物品，尝试堆叠
    if (canStackItem(item)) {
        auto existingItem = findStackableItem(item);
        if (existingItem) {
            existingItem->setQuantity(existingItem->getQuantity() + item->getQuantity());
            notifyItemChange(item->getName(), item->getQuantity(), true);
            return InventoryResult::SUCCESS;
        }
    }

    // 添加新物品
    items_.push_back(item);
    notifyItemChange(item->getName(), item->getQuantity(), true);
    return InventoryResult::SUCCESS;
}

InventoryResult Inventory::removeItem(const std::string& itemName, int quantity) {
    auto it = std::find_if(items_.begin(), items_.end(),
        [&itemName](const std::shared_ptr<Item>& item) {
            return item->getName() == itemName;
        });

    if (it == items_.end()) {
        return InventoryResult::NOT_FOUND;
    }

    if ((*it)->getQuantity() < quantity) {
        return InventoryResult::INSUFFICIENT_QUANTITY;
    }

    if ((*it)->getQuantity() == quantity) {
        // 移除整个物品
        items_.erase(it);
    } else {
        // 减少数量
        (*it)->setQuantity((*it)->getQuantity() - quantity);
    }

    notifyItemChange(itemName, quantity, false);
    return InventoryResult::SUCCESS;
}

InventoryResult Inventory::useItem(const std::string& itemName) {
    return removeItem(itemName, 1);
}

std::shared_ptr<Item> Inventory::getItem(const std::string& itemName) const {
    auto it = std::find_if(items_.begin(), items_.end(),
        [&itemName](const std::shared_ptr<Item>& item) {
            return item->getName() == itemName;
        });

    return (it != items_.end()) ? *it : nullptr;
}

std::vector<std::shared_ptr<Item>> Inventory::getAllItems() const {
    return items_;
}

std::vector<std::shared_ptr<Item>> Inventory::getItemsByType(ItemType type) const {
    std::vector<std::shared_ptr<Item>> result;
    for (const auto& item : items_) {
        if (item->getType() == type) {
            result.push_back(item);
        }
    }
    return result;
}

std::vector<std::shared_ptr<Item>> Inventory::getItemsByRarity(Rarity rarity) const {
    std::vector<std::shared_ptr<Item>> result;
    for (const auto& item : items_) {
        if (item->getRarity() == rarity) {
            result.push_back(item);
        }
    }
    return result;
}

void Inventory::sortByName(bool ascending) {
    std::sort(items_.begin(), items_.end(),
        [ascending](const std::shared_ptr<Item>& a, const std::shared_ptr<Item>& b) {
            if (ascending) {
                return a->getName() < b->getName();
            } else {
                return a->getName() > b->getName();
            }
        });
}

void Inventory::sortByType() {
    std::sort(items_.begin(), items_.end(),
        [](const std::shared_ptr<Item>& a, const std::shared_ptr<Item>& b) {
            return static_cast<int>(a->getType()) < static_cast<int>(b->getType());
        });
}

void Inventory::sortByRarity(bool ascending) {
    std::sort(items_.begin(), items_.end(),
        [ascending](const std::shared_ptr<Item>& a, const std::shared_ptr<Item>& b) {
            if (ascending) {
                return static_cast<int>(a->getRarity()) < static_cast<int>(b->getRarity());
            } else {
                return static_cast<int>(a->getRarity()) > static_cast<int>(b->getRarity());
            }
        });
}

std::vector<std::shared_ptr<Item>> Inventory::searchItems(const std::string& keyword) const {
    std::vector<std::shared_ptr<Item>> result;
    std::string lowerKeyword = keyword;
    std::transform(lowerKeyword.begin(), lowerKeyword.end(), lowerKeyword.begin(), ::tolower);

    for (const auto& item : items_) {
        std::string itemName = item->getName();
        std::transform(itemName.begin(), itemName.end(), itemName.begin(), ::tolower);

        std::string itemDesc = item->getDescription();
        std::transform(itemDesc.begin(), itemDesc.end(), itemDesc.begin(), ::tolower);

        if (itemName.find(lowerKeyword) != std::string::npos ||
            itemDesc.find(lowerKeyword) != std::string::npos) {
            result.push_back(item);
        }
    }
    return result;
}

std::unordered_map<ItemType, int> Inventory::getItemTypeCounts() const {
    std::unordered_map<ItemType, int> counts;
    for (const auto& item : items_) {
        counts[item->getType()]++;
    }
    return counts;
}

std::unordered_map<Rarity, int> Inventory::getRarityCounts() const {
    std::unordered_map<Rarity, int> counts;
    for (const auto& item : items_) {
        counts[item->getRarity()]++;
    }
    return counts;
}

InventoryResult Inventory::addItems(const std::vector<std::shared_ptr<Item>>& items) {
    for (const auto& item : items) {
        InventoryResult result = addItem(item);
        if (result != InventoryResult::SUCCESS) {
            return result; // 如果添加任何一个物品失败，整个操作失败
        }
    }
    return InventoryResult::SUCCESS;
}

std::vector<std::shared_ptr<Item>> Inventory::removeAllItems() {
    std::vector<std::shared_ptr<Item>> removedItems = items_;
    items_.clear();
    return removedItems;
}

void Inventory::notifyItemChange(const std::string& itemName, int quantity, bool added) {
    if (itemChangeCallback_) {
        itemChangeCallback_(itemName, quantity, added);
    }
}

bool Inventory::canStackItem(const std::shared_ptr<Item>& item) const {
    // 只有材料类型的物品可以堆叠
    if (item->getType() != ItemType::MATERIAL) {
        return false;
    }

    // 检查物品是否可堆叠
    auto material = std::dynamic_pointer_cast<Material>(item);
    return material && material->isStackable();
}

std::shared_ptr<Item> Inventory::findStackableItem(const std::shared_ptr<Item>& item) const {
    for (const auto& existingItem : items_) {
        if (existingItem->getName() == item->getName() &&
            existingItem->getType() == item->getType() &&
            existingItem->getRarity() == item->getRarity()) {
            return existingItem;
        }
    }
    return nullptr;
}
