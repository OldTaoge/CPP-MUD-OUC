#ifndef CPP_MUD_OUC_MAP_HPP
#define CPP_MUD_OUC_MAP_HPP
#include "../display.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <string>
#include <vector>
#include <memory>

// 前向声明
class Game;

class MapScreen : public BaseScreen {
public:
    MapScreen(Game* game = nullptr);
    ~MapScreen() = default;

    // 实现基类的虚函数
    ftxui::Component GetComponent() override;

    // 地图状态更新方法
    void UpdateMapData(const Game& game);
    void AddMapMessage(const std::string& message);
    void ClearMapMessages();

private:
    void HandleMovement(int deltaX, int deltaY);
    void HandleInteraction();
    void ShowInteractionMenu();
    void HideInteractionMenu();
    void HandleInteractionOption(int optionIndex);
    void UpdateMapDisplay();
    
    ftxui::Component component_;
    
    // UI组件
    ftxui::Component map_display_;
    ftxui::Component player_info_;
    ftxui::Component interaction_menu_;
    ftxui::Component close_button_;
    std::vector<ftxui::Component> interaction_buttons_;
    
    // 数据存储
    std::vector<std::string> map_lines_;
    std::vector<std::string> map_messages_;
    std::vector<std::string> interaction_options_;
    
    // 游戏状态
    std::string player_name_;
    int player_x_, player_y_;
    std::string current_block_info_;
    std::string player_status_;
    
    // 选择状态
    int selected_interaction_ = 0;
    bool show_interaction_menu_ = false;
    
    // 游戏对象引用
    Game* game_;
    
    // 地图尺寸
    static constexpr int MAP_WIDTH = 20;
    static constexpr int MAP_HEIGHT = 10;
};

#endif //CPP_MUD_OUC_MAP_HPP
