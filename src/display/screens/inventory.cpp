#include "inventory.hpp"
#include "../../player/player.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/event.hpp>
#include <algorithm>
#include <iostream>

using namespace ftxui;

InventoryScreen::InventoryScreen(Player* player) : player_(player) {
    // 创建返回按钮
    back_button_ = Button("返回游戏", [this] {
        this->ReturnToGame();
    });
    
    // 创建物品列表组件
    item_list_ = Container::Vertical({});
    
    // 创建物品详情相关按钮
    use_button_ = Button("使用", [this] {
        if (selected_item_index_ >= 0 && selected_item_index_ < static_cast<int>(player_->inventory.size())) {
            this->UseItem(selected_item_index_);
        }
    });
    
    drop_button_ = Button("丢弃", [this] {
        if (selected_item_index_ >= 0 && selected_item_index_ < static_cast<int>(player_->inventory.size())) {
            this->DropItem(selected_item_index_);
        }
    });
    
    close_detail_button_ = Button("关闭详情", [this] {
        this->HideItemDetails();
    });
    
    // 创建物品详情叠加层
    item_detail_overlay_ = Container::Vertical({use_button_, drop_button_, close_detail_button_});
    
    // 创建主组件
    component_ = Container::Vertical({back_button_, item_list_, item_detail_overlay_});
    
    // 更新物品栏
    UpdateInventory();
    
    // 设置渲染器
    component_ = Renderer(component_, [this] {
        // 构建物品列表元素
        Elements item_elements;
        for (const auto& item : player_->inventory) {
            std::string itemText = item.name;
            if (item.isStackable) {
                itemText += " (x" + std::to_string(item.stackSize) + ")";
            }
            itemText += " - " + item.getTypeString();
            item_elements.push_back(text(itemText) | color(Color::White));
        }
        
        // 构建物品列表区域
        auto inventory_area = vbox({
            text("物品栏") | bold | color(Color::Magenta) | hcenter,
            separator(),
            vbox({
                vbox(item_elements) | flex | border
            })
        }) | flex;
        
        // 构建底部按钮区域
        auto bottom_area = hbox({
            back_button_->Render() | hcenter | flex
        });
        
        // 基础界面
        auto base_interface = vbox({
            inventory_area | flex,
            separator(),
            bottom_area
        }) | border;
        
        // 物品详情叠加层
        if (show_item_detail_ && selected_item_index_ >= 0 && selected_item_index_ < static_cast<int>(player_->inventory.size())) {
            const Item& selectedItem = player_->inventory[selected_item_index_];
            
            Elements detail_elements;
            detail_elements.push_back(text("物品详情") | bold | color(Color::Yellow) | hcenter);
            detail_elements.push_back(separator());
            detail_elements.push_back(text("名称: " + selectedItem.name) | color(Color::White));
            detail_elements.push_back(text("类型: " + selectedItem.getTypeString()) | color(Color::White));
            detail_elements.push_back(text("描述: " + selectedItem.description) | color(Color::White));
            
            // 根据物品类型添加不同的属性显示
            if (selectedItem.type == ItemType::WEAPON) {
                detail_elements.push_back(text("攻击力: +" + std::to_string(selectedItem.attackBonus)) | color(Color::Red));
            } else if (selectedItem.type == ItemType::ARMOR) {
                detail_elements.push_back(text("防御力: +" + std::to_string(selectedItem.defenseBonus)) | color(Color::Blue));
            } else if (selectedItem.type == ItemType::CONSUMABLE) {
                if (selectedItem.healthRestore > 0) {
                    detail_elements.push_back(text("生命恢复: +" + std::to_string(selectedItem.healthRestore)) | color(Color::Green));
                }
            }
            
            detail_elements.push_back(text("价值: " + std::to_string(selectedItem.value) + " 金币") | color(Color::White));
            
            if (selectedItem.isStackable) {
                detail_elements.push_back(text("数量: " + std::to_string(selectedItem.stackSize)) | color(Color::White));
            }
            
            detail_elements.push_back(separator());
            
            // 添加操作按钮
            if (selectedItem.type == ItemType::CONSUMABLE) {
                detail_elements.push_back(use_button_->Render());
            } else {
                detail_elements.push_back(text("无法使用该物品") | color(Color::GrayLight));
            }
            
            detail_elements.push_back(drop_button_->Render());
            detail_elements.push_back(close_detail_button_->Render() | hcenter);
            
            auto overlay_element = vbox(detail_elements) | border | bgcolor(Color::DarkBlue) | color(Color::White);
            
            return overlay_element | hcenter | vcenter;
        } else {
            return base_interface;
        }
    });
}


Component InventoryScreen::GetComponent() {
    return component_;
}

void InventoryScreen::UpdateInventory() {
    // 清空现有物品按钮
    item_buttons_.clear();
    item_list_->DetachAllChildren();
    
    // 为每个物品创建按钮
    for (size_t i = 0; i < player_->inventory.size(); ++i) {
        auto button = Button(player_->inventory[i].name, [this, i] { 
            this->ShowItemDetails(i); 
        });
        item_buttons_.push_back(button);
        item_list_->Add(button);
    }
}

void InventoryScreen::HandleItemAction(int itemIndex, const std::string& action) {
    if (itemIndex < 0 || itemIndex >= static_cast<int>(player_->inventory.size())) {
        return;
    }
    
    if (action == "use") {
        UseItem(itemIndex);
    } else if (action == "drop") {
        DropItem(itemIndex);
    }
}

void InventoryScreen::ShowItemDetails(int itemIndex) {
    selected_item_index_ = itemIndex;
    show_item_detail_ = true;
}

void InventoryScreen::HideItemDetails() {
    show_item_detail_ = false;
    selected_item_index_ = -1;
}

void InventoryScreen::UseItem(int itemIndex) {
    if (player_->useItem(itemIndex)) {
        // 物品使用成功后的逻辑，例如恢复生命值
        if (itemIndex < static_cast<int>(player_->inventory.size())) {
            const Item& usedItem = player_->inventory[itemIndex];
            if (usedItem.healthRestore > 0) {
                // 恢复玩家生命值，假设最大生命值为100
                player_->health = std::min(player_->health + usedItem.healthRestore, 100);
                // 可以在这里添加提示信息，告知玩家生命值已恢复
            }
        }
        
        // 更新物品栏
        UpdateInventory();
        HideItemDetails();
    }
}

void InventoryScreen::DropItem(int itemIndex) {
    if (player_->removeItem(itemIndex)) {
        // 更新物品栏
        UpdateInventory();
        HideItemDetails();
    }
}

void InventoryScreen::ReturnToGame() {
    // 切换回游戏界面
    if (navigation_callback_) {
        navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Gameplay"));
    }
}