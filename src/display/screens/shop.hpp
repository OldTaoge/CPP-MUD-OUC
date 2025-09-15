// =============================================
// 文件: shop.hpp
// 描述: 商店界面。使用材料兑换武器，支持键盘与鼠标操作。
// =============================================
#pragma once
#include "../display.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>
#include <vector>
#include <functional>
#include "../../core/game.h"
#include "../../core/item.h"

class Game;

class ShopScreen : public BaseScreen {
public:
    explicit ShopScreen(Game* game) : game_(game) {
        using namespace ftxui;
        status_message_ = "欢迎光临！使用素材兑换武器。";

        // 垂直菜单（上下选择），Enter 确认
        menu_entries_.clear();
        selected_index_ = 0;
        MenuOption opt;
        opt.on_enter = [this] { ConfirmPurchase(); };
        list_menu_ = Menu(&menu_entries_, &selected_index_, opt);
        confirm_btn_ = Button("确认兑换 (Enter)", [this] { ConfirmPurchase(); });
        root_container_ = Container::Vertical({ list_menu_, confirm_btn_ });

        // 主渲染器（根据材料动态展示可用状态与提示）
        component_ = Renderer(root_container_, [this] {
            using namespace ftxui;
            std::vector<Element> lines;
            lines.push_back(text("≋≋≋ 商 店 ≋≋≋") | bold | color(Color::Yellow) | center);
            lines.push_back(separator());
            lines.push_back(text("材料需求：") | bold | color(Color::Cyan));
            auto counts = RenderMaterialCounts();
            lines.insert(lines.end(), counts.begin(), counts.end());
            lines.push_back(separator());

            // 构建列表与确认按钮
            BuildMenuEntries();
            lines.push_back(text("可交易条目：") | bold | color(Color::Yellow));
            lines.push_back(list_menu_->Render());
            lines.push_back(separator());
            lines.push_back(confirm_btn_->Render());
            lines.push_back(separator());

            // 状态提示颜色
            ftxui::Color msg_color = Color::White;
            if (status_message_.find("成功") != std::string::npos) msg_color = Color::Green;
            if (status_message_.find("不足") != std::string::npos) msg_color = Color::Red;
            lines.push_back(text(status_message_) | color(msg_color));
            lines.push_back(text("提示：↑↓ 选择  Enter 兑换  B 返回  亦支持鼠标点击") | color(Color::GrayLight));
            return vbox(lines) | border | color(Color::Cyan);
        });

        // 键盘事件：B 返回 / Enter 确认
        component_ |= ftxui::CatchEvent([this](ftxui::Event e) {
            if (e == ftxui::Event::Character('b') || e == ftxui::Event::Character('B')) {
                if (navigation_callback_) {
                    navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Gameplay"));
                }
                return true;
            } else if (e == ftxui::Event::Return) {
                ConfirmPurchase();
                return true;
            }
            return false;
        });
    }

    ftxui::Component GetComponent() override { return component_; }

private:
    // 条目构建与动作
    void BuildMenuEntries();
    void ConfirmPurchase();

    std::vector<ftxui::Element> RenderMaterialCounts();
    void HandleBuyTwoStar();
    void HandleBuyThreeStar();
    bool ConsumeMaterial(const std::string& name);
    void GiveWeapon(const std::string& name, Rarity rarity);
    int CountItem(const std::string& name);

    Game* game_;
    ftxui::Component component_;
    ftxui::Component root_container_;
    ftxui::Component list_menu_;
    ftxui::Component confirm_btn_;
    std::vector<std::string> menu_entries_;
    int selected_index_ = 0;
    std::string status_message_;
};

// ==== Inline implementations (simple glue with game/inventory) ====
inline std::vector<ftxui::Element> ShopScreen::RenderMaterialCounts() {
    using namespace ftxui;
    std::vector<Element> out;
    if (!game_) return out;
    const auto& items = game_->getPlayer().inventory.getAllItems();
    int slime_condensate = 0;   // 1★ 史莱姆凝液
    int slime_secretions = 0;   // 2★ 史莱姆原浆
    for (auto& it : items) {
        if (it->getName() == "史莱姆凝液") slime_condensate += it->getQuantity();
        if (it->getName() == "史莱姆原浆") slime_secretions += it->getQuantity();
    }
    out.push_back(text("库存：1★ 史莱姆凝液 ×" + std::to_string(slime_condensate)) | color(Color::Green));
    out.push_back(text("库存：2★ 史莱姆原浆 ×" + std::to_string(slime_secretions)) | color(Color::Yellow));
    return out;
}

inline int ShopScreen::CountItem(const std::string& name) {
    int total = 0;
    const auto& items = game_->getPlayer().inventory.getAllItems();
    for (auto& it : items) if (it->getName() == name) total += it->getQuantity();
    return total;
}

inline bool ShopScreen::ConsumeMaterial(const std::string& name) {
    auto& inv = game_->getPlayer().inventory;
    if (CountItem(name) <= 0) return false;
    auto res = inv.removeItem(name, 1);
    return res == InventoryResult::SUCCESS;
}

inline void ShopScreen::GiveWeapon(const std::string& name, Rarity rarity) {
    auto weapon = ItemFactory::createWeapon(name, WeaponType::ONE_HANDED_SWORD, rarity);
    weapon->setQuantity(1);
    game_->getPlayer().addItemToInventory(weapon);
}

inline void ShopScreen::HandleBuyTwoStar() {
    // 1★ 史莱姆凝液 → 2★ 武器
    if (!ConsumeMaterial("史莱姆凝液")) {
        status_message_ = "材料不足：需要 1★ 史莱姆凝液 ×1";
        return;
    }
    GiveWeapon("铁影阔剑", Rarity::TWO_STAR);
    status_message_ = "兑换成功：获得 2★ 武器『铁影阔剑』";
}

inline void ShopScreen::HandleBuyThreeStar() {
    // 2★ 史莱姆原浆 → 3★ 武器
    if (!ConsumeMaterial("史莱姆原浆")) {
        status_message_ = "材料不足：需要 2★ 史莱姆原浆 ×1";
        return;
    }
    GiveWeapon("飞天御剑", Rarity::THREE_STAR);
    status_message_ = "兑换成功：获得 3★ 武器『飞天御剑』";
}

inline void ShopScreen::BuildMenuEntries() {
    menu_entries_.clear();
    bool can_buy_2 = CountItem("史莱姆凝液") >= 1;
    bool can_buy_3 = CountItem("史莱姆原浆") >= 1;

    // 使用颜色标签增强可视：可购买项前缀加上指示
    std::string entry2 = std::string(can_buy_2 ? "[可兑换] " : "[材料不足] ") + "2★ 铁影阔剑  | 消耗：1★ 史莱姆凝液 ×1";
    std::string entry3 = std::string(can_buy_3 ? "[可兑换] " : "[材料不足] ") + "3★ 飞天御剑  | 消耗：2★ 史莱姆原浆 ×1";
    menu_entries_.push_back(entry2);
    menu_entries_.push_back(entry3);

    // 防止越界
    if (selected_index_ < 0) selected_index_ = 0;
    if (selected_index_ >= (int)menu_entries_.size()) selected_index_ = (int)menu_entries_.size() - 1;
}

inline void ShopScreen::ConfirmPurchase() {
    if (selected_index_ == 0) {
        HandleBuyTwoStar();
    } else if (selected_index_ == 1) {
        HandleBuyThreeStar();
    }
}



