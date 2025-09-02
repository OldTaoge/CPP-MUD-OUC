#ifndef CPP_MUD_OUC_INVENTORY_HPP
#define CPP_MUD_OUC_INVENTORY_HPP
#include "../display.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <string>
#include <vector>
#include <memory>
#include "../../player/item.h"

// 前向声明
class Player;

class InventoryScreen : public BaseScreen {
public:
    InventoryScreen(Player* player);
    ~InventoryScreen() = default;

    // 实现基类的虚函数
    ftxui::Component GetComponent() override;
    
    // 更新物品栏信息
    void UpdateInventory();
    
    // 处理物品操作
    void HandleItemAction(int itemIndex, const std::string& action);

private:
    // 显示物品详情
    void ShowItemDetails(int itemIndex);
    
    // 关闭物品详情
    void HideItemDetails();
    
    // 玩家对象指针
    Player* player_;
    
    // 使用物品
    void UseItem(int itemIndex);
    
    // 丢弃物品
    void DropItem(int itemIndex);
    
    // 返回游戏界面
    void ReturnToGame();

    ftxui::Component component_;
    
    // UI组件
    ftxui::Component back_button_;
    ftxui::Component item_list_;
    std::vector<ftxui::Component> item_buttons_;
    ftxui::Component item_detail_overlay_;
    ftxui::Component use_button_;
    ftxui::Component drop_button_;
    ftxui::Component close_detail_button_;
    

    
    // 状态
    int selected_item_index_ = -1;
    bool show_item_detail_ = false;
    int selected_action_ = 0;
};

#endif //CPP_MUD_OUC_INVENTORY_HPP