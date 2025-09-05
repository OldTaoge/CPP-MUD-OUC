#pragma once
#include "../display.hpp"
#include "../../core/game.h"
#include "../../core/item.h"
#include <vector>
#include <string>

class InventoryScreen : public BaseScreen {
public:
    InventoryScreen(Game* game);
    ~InventoryScreen() = default;

    // 实现基类的虚函数
    ftxui::Component GetComponent() override;

private:
    Game* game_;
    ftxui::Component component_;

    // UI 状态
    std::vector<std::shared_ptr<Item>> currentItems_;
    std::string selectedItemName_;
    std::string filterType_;
    std::string searchKeyword_;
    int selectedIndex_;
    int selectedMemberIndex_;
    bool showItemDetails_;
    bool showMemberSelection_;

    // UI 组件
    ftxui::Component itemList_;
    ftxui::Component itemDetails_;
    ftxui::Component filterButtons_;
    ftxui::Component actionButtons_;

    // 初始化函数
    void InitializeItems();
    void UpdateItemList();
    void CreateItemList();
    void CreateItemDetails();
    void CreateFilterButtons();
    void CreateActionButtons();

    // 事件处理
    void HandleItemSelection(int index);
    void HandleFilterSelection(const std::string& filter);
    void HandleAction(int action);
    void HandleSearch(const std::string& keyword);

    // 辅助函数
    std::string GetItemDisplayText(const std::shared_ptr<Item>& item) const;
    std::vector<std::shared_ptr<Item>> GetFilteredItems() const;
    std::string GetInventoryStats() const;
};
