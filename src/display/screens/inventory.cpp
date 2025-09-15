// =============================================
// 文件: inventory.cpp
// 描述: 背包界面实现。列表渲染、筛选搜索、使用/装备与状态提示。
// =============================================
#include "inventory.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
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
    CreateSearchInput();

    // 首次进入时同步一次筛选结果，确保有选中项与物品名
    UpdateItemList();

    // 队伍成员选择组件
    auto memberSelectionCore = Renderer([this] {
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
    memberSelection_ = CatchEvent(memberSelectionCore, [this](Event e) {
        if (!showMemberSelection_) return false;
        auto teamMembers = game_->getPlayer().getTeamMembers();
        int count = static_cast<int>(teamMembers.size());
        if (count == 0) return false;

        if (e == Event::ArrowUp) {
            selectedMemberIndex_ = (selectedMemberIndex_ - 1 + count) % count;
            return true;
        }
        if (e == Event::ArrowDown) {
            selectedMemberIndex_ = (selectedMemberIndex_ + 1) % count;
            return true;
        }
        if (e == Event::Escape) {
            showMemberSelection_ = false;
            return true;
        }
        if (e == Event::Return) {
            // 直接在此执行装备确认逻辑
            if (!currentItems_.empty() && selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(currentItems_.size())) {
                const auto& item = currentItems_[selectedIndex_];
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
            }
            return true;
        }
        return false;
    });

    // 创建主布局
    auto horizontal_container = Container::Horizontal({itemList_, itemDetails_});
    
    auto main_layout = Container::Vertical({
        filterButtons_,
        searchInput_,
        horizontal_container,
        memberSelection_,
        actionButtons_
    });

    auto renderer = Renderer(main_layout, [this, main_layout] {
        return vbox({
            text("🎒 背包系统") | bold | hcenter | color(Color::Cyan),
            separator(),
            hbox({
                text("当前筛选: ") | dim, text(filterType_) | color(Color::Blue) | bold,
                filler(),
                text("容量: ") | dim, text(GetInventoryStats()) | color(Color::Yellow)
            }),
            separator(),
            // 状态提示区域
            (status_message_.empty() ? text("") : text(status_message_) | color(Color::Red)) ,
            separator(),
            main_layout->Render() | flex
        }) | border;
    });

    component_ = renderer;
    // 全局按键转发到物品列表，保证可用性
    component_ = CatchEvent(component_, [this](Event e) {
        // 成员选择弹窗打开时，优先把上下/确认/取消转给成员选择
        if (showMemberSelection_) {
            if (e == Event::ArrowUp || e == Event::ArrowDown || e == Event::Escape || e == Event::Return) {
                if (memberSelection_) return memberSelection_->OnEvent(e);
                return true;
            }
            return false;
        }
        if (e == Event::ArrowUp || e == Event::ArrowDown ||
            e == Event::PageUp || e == Event::PageDown ||
            e == Event::Home || e == Event::End ||
            e == Event::Return) {
            if (itemList_) {
                return itemList_->OnEvent(e);
            }
        }
        return false;
    });
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
    auto item_list_renderer_core = Renderer([this] {
        Elements item_elements;
        auto filteredItems = GetFilteredItems();

        if (filteredItems.empty()) {
            item_elements.push_back(text("背包为空") | dim);
        } else {
            for (size_t i = 0; i < filteredItems.size(); ++i) {
                const auto& item = filteredItems[i];
                std::string displayText = GetItemDisplayText(item);

                // 稀有度着色与类型图标
                Color rarity_color = Color::White;
                auto rarity = item->getRarityString();
                if (rarity.find("传说") != std::string::npos || rarity.find("5") != std::string::npos) rarity_color = Color::Yellow;
                else if (rarity.find("史诗") != std::string::npos || rarity.find("4") != std::string::npos) rarity_color = Color::Magenta;
                else if (rarity.find("稀有") != std::string::npos || rarity.find("3") != std::string::npos) rarity_color = Color::Blue;
                else if (rarity.find("精良") != std::string::npos || rarity.find("2") != std::string::npos) rarity_color = Color::Green;
                else rarity_color = Color::GrayLight;

                std::string icon = "";
                switch (item->getType()) {
                    case ItemType::WEAPON: icon = "🗡 "; break;
                    case ItemType::ARTIFACT: icon = "🔮 "; break;
                    case ItemType::FOOD: icon = "🍗 "; break;
                    case ItemType::MATERIAL: icon = "🧱 "; break;
                    default: break;
                }

                auto item_element = text(icon + displayText) | color(rarity_color);
                if (i == static_cast<size_t>(selectedIndex_)) {
                    item_element = item_element | inverted;
                }

                item_elements.push_back(item_element);
            }
        }

        return window(text("物品列表"), vbox(std::move(item_elements)) | frame | size(WIDTH, GREATER_THAN, 30));
    });
    itemList_ = CatchEvent(item_list_renderer_core, [this](Event e) {
        auto items = GetFilteredItems();
        int n = static_cast<int>(items.size());
        if (n == 0) return false;

        if (e == Event::ArrowUp) {
            selectedIndex_ = (selectedIndex_ - 1 + n) % n;
        } else if (e == Event::ArrowDown) {
            selectedIndex_ = (selectedIndex_ + 1) % n;
        } else if (e == Event::PageUp) {
            selectedIndex_ = std::max(0, selectedIndex_ - 5);
        } else if (e == Event::PageDown) {
            selectedIndex_ = std::min(n - 1, selectedIndex_ + 5);
        } else if (e == Event::Home) {
            selectedIndex_ = 0;
        } else if (e == Event::End) {
            selectedIndex_ = n - 1;
        } else if (e == Event::Return) {
            // 查看详情
            if (selectedIndex_ >= 0 && selectedIndex_ < n) {
                selectedItemName_ = items[selectedIndex_]->getName();
                showItemDetails_ = true;
            }
        } else {
            return false;
        }
        // 同步当前选中名称，便于动作按钮使用
        if (selectedIndex_ >= 0 && selectedIndex_ < n) {
            selectedItemName_ = items[selectedIndex_]->getName();
        }
        return true;
    });
}

void InventoryScreen::CreateItemDetails() {
    itemDetails_ = Renderer([this] {
        auto filteredItems = GetFilteredItems();
        if (filteredItems.empty()) {
            return window(text("物品详情"), vbox({
                text("背包为空") | dim | hcenter,
            }) | size(WIDTH, GREATER_THAN, 40));
        }

        int index = selectedIndex_;
        if (index < 0) index = 0;
        if (index >= static_cast<int>(filteredItems.size())) index = static_cast<int>(filteredItems.size()) - 1;
        const auto& item = filteredItems[index];

        Elements details;
        details.push_back(text("📦 物品详情") | bold | hcenter | color(Color::Cyan));
        details.push_back(separator());
        details.push_back(text("名称: " + item->getName()));
        details.push_back(text("类型: " + item->getTypeString()));
        details.push_back(text("稀有度: " + item->getRarityString()));
        details.push_back(text("数量: " + std::to_string(item->getQuantity())));

        std::vector<std::string> detailLines;
        std::string detailedInfo = item->getDetailedInfo();
        std::stringstream ss(detailedInfo);
        std::string line;
        while (std::getline(ss, line)) {
            if (!line.empty()) {
                detailLines.push_back(line);
            }
        }

        for (const auto& dl : detailLines) {
            details.push_back(text(dl));
        }

        return window(text("物品详情"), vbox(std::move(details)) | size(WIDTH, GREATER_THAN, 40));
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

    // 渲染为带标题的窗口
    auto container = Container::Horizontal(buttons);
    filterButtons_ = Renderer(container, [this, container] {
        return window(text("筛选"), container->Render());
    });
}

void InventoryScreen::CreateActionButtons() {
    std::vector<Component> buttons;

    // 使用物品按钮
    auto useButton = Button("使用物品", [this] {
        auto items = GetFilteredItems();
        int n = static_cast<int>(items.size());
        if (n == 0) return;
        int idx = std::clamp(selectedIndex_, 0, n - 1);
        selectedItemName_ = items[idx]->getName();
        auto result = game_->getPlayer().useItem(selectedItemName_);
        switch (result) {
            case InventoryResult::SUCCESS:
                status_message_ = "使用成功";
                UpdateItemList();
                break;
            case InventoryResult::INVALID_OPERATION:
                status_message_ = "该物品不能被使用";
                break;
            case InventoryResult::NOT_FOUND:
                status_message_ = "未找到该物品";
                break;
            case InventoryResult::INSUFFICIENT_QUANTITY:
                status_message_ = "数量不足，无法使用";
                break;
            case InventoryResult::FULL:
                status_message_ = "背包已满"; // 理论上不出现在使用时
                break;
        }
    });
    buttons.push_back(useButton);

    // 装备物品按钮
    auto equipButton = Button("装备", [this] {
        auto items = GetFilteredItems();
        int n = static_cast<int>(items.size());
        if (n == 0) return;
        int idx = std::clamp(selectedIndex_, 0, n - 1);
        const auto& item = items[idx];
        selectedItemName_ = item->getName();

        if (item->getType() != ItemType::WEAPON && item->getType() != ItemType::ARTIFACT) {
            status_message_ = "该物品不能被装备";
            return;
        }

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
                status_message_ = "装备成功";
                UpdateItemList();
            } else {
                status_message_ = "装备失败";
            }
        } else {
            // 开始选择队伍成员
            showMemberSelection_ = true;
            selectedMemberIndex_ = 0; // 重置选择
            status_message_.clear();
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

    {
        auto inner = Container::Horizontal(buttons);
        actionButtons_ = Renderer(inner, [inner] { return window(text("操作"), inner->Render()); });
    }
}

void InventoryScreen::CreateSearchInput() {
    // 输入搜索关键字，回车立即生效
    auto input = Input(&searchKeyword_, "输入搜索关键字...（回车确认）");
    auto core = CatchEvent(input, [this](Event e) {
        if (e == Event::Return) {
            HandleSearch(searchKeyword_);
            return true;
        }
        return false;
    });
    searchInput_ = Renderer(core, [core] { return window(text("搜索"), core->Render()); });
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
