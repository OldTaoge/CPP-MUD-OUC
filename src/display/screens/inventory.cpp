#include "inventory.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <sstream>
#include <algorithm>
#include "../../utils/utils.hpp"

using namespace ftxui;

InventoryScreen::InventoryScreen(Game* game)
    : game_(game), selectedIndex_(0), selectedMemberIndex_(0),
      showItemDetails_(false), showMemberSelection_(false),
      filterType_("全部"), searchKeyword_(""), selectedItemName_("") {
    InitializeItems();
    CreateItemList();
    CreateItemDetails();
    CreateFilterButtons();
    CreateActionButtons();

    // 队伍成员选择组件
    auto memberSelection = Renderer([this] {
        if (!showMemberSelection_) {
            return text("") | size(WIDTH, EQUAL, 0) | size(HEIGHT, EQUAL, 0);
        }

        auto teamMembers = game_->getPlayer().getTeamMembers();
        Elements memberElements;

        memberElements.push_back(text("选择要装备的队伍成员:") | bold | hcenter);
        memberElements.push_back(separator());

        for (size_t i = 0; i < teamMembers.size(); ++i) {
            const auto& member = teamMembers[i];
            std::string displayText = std::to_string(i + 1) + ". " + member->getName() +
                                    " (Lv." + std::to_string(member->getLevel()) + ")";

            if (member->getEquippedWeapon()) {
                displayText += " [武器: " + member->getEquippedWeapon()->getName() + "]";
            }
            if (member->getEquippedArtifact()) {
                displayText += " [圣遗物: " + member->getEquippedArtifact()->getName() + "]";
            }

            auto element = text(displayText);
            if (static_cast<int>(i) == selectedMemberIndex_) {
                element = element | inverted;
            }

            memberElements.push_back(element);
        }

        memberElements.push_back(separator());
        memberElements.push_back(text("按Enter确认装备，ESC取消") | dim | hcenter);

        return vbox(std::move(memberElements)) | border | size(WIDTH, GREATER_THAN, 50);
    });

    // 创建主布局
    auto horizontal_container = Container::Horizontal({itemList_, itemDetails_});
    
    auto main_layout = Container::Vertical({
        filterButtons_,
        horizontal_container,
        memberSelection,
        actionButtons_
    });

    auto renderer = Renderer(main_layout, [this, main_layout] {
        return vbox({
            text("🎒 背包系统") | bold | hcenter,
            separator(),
            hbox({
                text("筛选: " + filterType_) | color(Color::Blue),
                filler(),
                text("搜索: " + searchKeyword_) | color(Color::Green),
                filler(),
                text(GetInventoryStats()) | color(Color::Yellow)
            }),
            separator(),
            main_layout->Render() | flex
        }) | border;
    });

    component_ = renderer;
}

Component InventoryScreen::GetComponent() {
    return component_;
}

void InventoryScreen::InitializeItems() {
    currentItems_ = game_->getPlayer().inventory.getAllItems();
    if (!currentItems_.empty()) {
        selectedItemName_ = currentItems_[0]->getName();
    }
}

void InventoryScreen::UpdateItemList() {
    currentItems_ = GetFilteredItems();
    if (selectedIndex_ >= static_cast<int>(currentItems_.size())) {
        selectedIndex_ = std::max(0, static_cast<int>(currentItems_.size()) - 1);
    }
    if (!currentItems_.empty() && selectedIndex_ < static_cast<int>(currentItems_.size())) {
        selectedItemName_ = currentItems_[selectedIndex_]->getName();
    } else {
        selectedItemName_ = "";
    }
    showItemDetails_ = !selectedItemName_.empty();
}

std::vector<std::shared_ptr<Item>> InventoryScreen::GetFilteredItems() const {
    auto allItems = game_->getPlayer().inventory.getAllItems();
    std::vector<std::shared_ptr<Item>> filtered;

    for (const auto& item : allItems) {
        bool typeMatch = (filterType_ == "全部");
        if (!typeMatch) {
            if (filterType_ == "武器" && item->getType() == ItemType::WEAPON) typeMatch = true;
            else if (filterType_ == "圣遗物" && item->getType() == ItemType::ARTIFACT) typeMatch = true;
            else if (filterType_ == "食物" && item->getType() == ItemType::FOOD) typeMatch = true;
            else if (filterType_ == "材料" && item->getType() == ItemType::MATERIAL) typeMatch = true;
        }

        bool searchMatch = searchKeyword_.empty();
        if (!searchMatch) {
            std::string itemName = item->getName();
            std::string itemDesc = item->getDescription();
            std::transform(itemName.begin(), itemName.end(), itemName.begin(), ::tolower);
            std::transform(itemDesc.begin(), itemDesc.end(), itemDesc.begin(), ::tolower);
            std::string keyword = searchKeyword_;
            std::transform(keyword.begin(), keyword.end(), keyword.begin(), ::tolower);
            searchMatch = (itemName.find(keyword) != std::string::npos ||
                          itemDesc.find(keyword) != std::string::npos);
        }

        if (typeMatch && searchMatch) {
            filtered.push_back(item);
        }
    }

    return filtered;
}

void InventoryScreen::CreateItemList() {
    auto item_list_renderer = Renderer([this] {
        Elements item_elements;
        auto filteredItems = GetFilteredItems();

        if (filteredItems.empty()) {
            item_elements.push_back(text("背包为空") | dim);
        } else {
            for (size_t i = 0; i < filteredItems.size(); ++i) {
                const auto& item = filteredItems[i];
                std::string displayText = GetItemDisplayText(item);

                auto item_element = text(displayText);
                if (i == static_cast<size_t>(selectedIndex_)) {
                    item_element = item_element | inverted;
                }

                item_elements.push_back(item_element);
            }
        }

        return vbox(std::move(item_elements)) | frame | size(WIDTH, GREATER_THAN, 30);
    });

    itemList_ = item_list_renderer;
}

void InventoryScreen::CreateItemDetails() {
    itemDetails_ = Renderer([this] {
        if (!showItemDetails_ || currentItems_.empty() ||
            selectedIndex_ >= static_cast<int>(currentItems_.size())) {
            return vbox({
                text("请选择物品") | dim | hcenter,
                text(""),
                text("使用方向键选择物品") | dim | hcenter,
                text("按Enter查看详情") | dim | hcenter
            }) | size(WIDTH, GREATER_THAN, 40);
        }

        const auto& item = currentItems_[selectedIndex_];
        Elements details;

        details.push_back(text("📦 物品详情") | bold | hcenter);
        details.push_back(separator());
        details.push_back(text("名称: " + item->getName()));
        details.push_back(text("类型: " + item->getTypeString()));
        details.push_back(text("稀有度: " + item->getRarityString()));
        details.push_back(text("数量: " + std::to_string(item->getQuantity())));

        // 根据物品类型显示额外信息
        std::vector<std::string> detailLines;
        std::string detailedInfo = item->getDetailedInfo();
        std::stringstream ss(detailedInfo);
        std::string line;
        while (std::getline(ss, line)) {
            if (!line.empty()) {
                detailLines.push_back(line);
            }
        }

        for (const auto& line : detailLines) {
            details.push_back(text(line));
        }

        return vbox(std::move(details)) | border | size(WIDTH, GREATER_THAN, 40);
    });
}

void InventoryScreen::CreateFilterButtons() {
    std::vector<std::string> filters = {"全部", "武器", "圣遗物", "食物", "材料"};

    std::vector<Component> buttons;
    for (const auto& filter : filters) {
        auto button = Button(filter, [this, filter] {
            filterType_ = filter;
            selectedIndex_ = 0;
            UpdateItemList();
        });
        buttons.push_back(button);
    }

    filterButtons_ = Container::Horizontal(buttons);
}

void InventoryScreen::CreateActionButtons() {
    std::vector<Component> buttons;

    // 使用物品按钮
    auto useButton = Button("使用物品", [this] {
        if (!selectedItemName_.empty()) {
            auto result = game_->getPlayer().useItem(selectedItemName_);
            if (result == InventoryResult::SUCCESS) {
                UpdateItemList();
            }
        }
    });
    buttons.push_back(useButton);

    // 装备物品按钮
    auto equipButton = Button("装备", [this] {
        if (!selectedItemName_.empty() && !currentItems_.empty() &&
            selectedIndex_ < static_cast<int>(currentItems_.size())) {
            const auto& item = currentItems_[selectedIndex_];

            if (showMemberSelection_) {
                // 确认装备到选中的成员
                bool success = false;
                if (item->getType() == ItemType::WEAPON) {
                    success = game_->getPlayer().equipWeaponForMember(selectedMemberIndex_, selectedItemName_);
                } else if (item->getType() == ItemType::ARTIFACT) {
                    success = game_->getPlayer().equipArtifactForMember(selectedMemberIndex_, selectedItemName_);
                }

                if (success) {
                    showMemberSelection_ = false;
                    UpdateItemList();
                }
            } else {
                // 开始选择队伍成员
                showMemberSelection_ = true;
                selectedMemberIndex_ = 0; // 重置选择
            }
        }
    });
    buttons.push_back(equipButton);

    // 丢弃物品按钮
    auto dropButton = Button("丢弃", [this] {
        if (!selectedItemName_.empty()) {
            auto result = game_->getPlayer().removeItemFromInventory(selectedItemName_, 1);
            if (result == InventoryResult::SUCCESS) {
                UpdateItemList();
            }
        }
    });
    buttons.push_back(dropButton);

    // 取消装备选择按钮
    auto cancelButton = Button("取消装备", [this] {
        showMemberSelection_ = false;
    });
    buttons.push_back(cancelButton);

    // 返回按钮
    auto backButton = Button("返回", [this] {
        if (navigation_callback_) {
            navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Gameplay"));
        }
    });
    buttons.push_back(backButton);

    actionButtons_ = Container::Horizontal(buttons);
}

std::string InventoryScreen::GetItemDisplayText(const std::shared_ptr<Item>& item) const {
    std::stringstream ss;
    ss << item->getRarityString() << " " << item->getName();

    if (item->getQuantity() > 1) {
        ss << " (x" << item->getQuantity() << ")";
    }

    return ss.str();
}

std::string InventoryScreen::GetInventoryStats() const {
    std::stringstream ss;
    ss << "物品: " << game_->getPlayer().inventory.getCurrentSize()
       << "/" << game_->getPlayer().inventory.getMaxCapacity();
    return ss.str();
}

void InventoryScreen::HandleItemSelection(int index) {
    if (index >= 0 && index < static_cast<int>(currentItems_.size())) {
        selectedIndex_ = index;
        selectedItemName_ = currentItems_[index]->getName();
        showItemDetails_ = true;
    }
}

void InventoryScreen::HandleFilterSelection(const std::string& filter) {
    filterType_ = filter;
    selectedIndex_ = 0;
    UpdateItemList();
}

void InventoryScreen::HandleAction(int action) {
    // 实现动作处理逻辑
}

void InventoryScreen::HandleSearch(const std::string& keyword) {
    searchKeyword_ = keyword;
    selectedIndex_ = 0;
    UpdateItemList();
}
