// =============================================
// 文件: inventory.h
// 描述: 背包系统声明。包含物品增删查用、筛选/排序与统计。
// =============================================
#pragma once
#include "item.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <functional>

// 背包操作结果枚举
enum class InventoryResult {
    SUCCESS,
    FULL,
    NOT_FOUND,
    INSUFFICIENT_QUANTITY,
    INVALID_OPERATION
};

// 背包类
class Inventory {
public:
    Inventory(size_t maxCapacity = 100);
    ~Inventory() = default;

    // 物品管理
    InventoryResult addItem(std::shared_ptr<Item> item);
    InventoryResult removeItem(const std::string& itemName, int quantity = 1);
    InventoryResult useItem(const std::string& itemName);
    std::shared_ptr<Item> getItem(const std::string& itemName) const;

    // 查询功能
    std::vector<std::shared_ptr<Item>> getAllItems() const;
    std::vector<std::shared_ptr<Item>> getItemsByType(ItemType type) const;
    std::vector<std::shared_ptr<Item>> getItemsByRarity(Rarity rarity) const;
    size_t getCurrentSize() const { return items_.size(); }
    size_t getMaxCapacity() const { return maxCapacity_; }
    bool isFull() const { return items_.size() >= maxCapacity_; }
    bool isEmpty() const { return items_.empty(); }

    // 排序功能
    void sortByName(bool ascending = true);
    void sortByType();
    void sortByRarity(bool ascending = true);

    // 搜索功能
    std::vector<std::shared_ptr<Item>> searchItems(const std::string& keyword) const;

    // 统计信息
    std::unordered_map<ItemType, int> getItemTypeCounts() const;
    std::unordered_map<Rarity, int> getRarityCounts() const;

    // 容量管理
    void setMaxCapacity(size_t capacity) { maxCapacity_ = capacity; }
    void expandCapacity(size_t additionalSlots) { maxCapacity_ += additionalSlots; }

    // 批量操作
    InventoryResult addItems(const std::vector<std::shared_ptr<Item>>& items);
    std::vector<std::shared_ptr<Item>> removeAllItems();

    // 回调函数类型，用于物品变化通知
    using ItemChangeCallback = std::function<void(const std::string& itemName, int quantity, bool added)>;

    // 设置物品变化回调
    void setItemChangeCallback(ItemChangeCallback callback) {
        itemChangeCallback_ = callback;
    }

private:
    std::vector<std::shared_ptr<Item>> items_;
    size_t maxCapacity_;
    ItemChangeCallback itemChangeCallback_;

    // 内部辅助函数
    void notifyItemChange(const std::string& itemName, int quantity, bool added);
    bool canStackItem(const std::shared_ptr<Item>& item) const;
    std::shared_ptr<Item> findStackableItem(const std::shared_ptr<Item>& item) const;
};
