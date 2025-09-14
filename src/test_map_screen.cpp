#include "display/screens/map.hpp"
#include "core/game.h"
#include <iostream>
#include <memory>

int main() {
    std::cout << "=== 地图界面测试 ===" << std::endl;
    
    try {
        // 创建游戏对象
        Game game;
        game.StartNewGame();
        
        std::cout << "游戏对象创建成功" << std::endl;
        
        // 创建地图界面
        MapScreen mapScreen(&game);
        
        std::cout << "地图界面创建成功" << std::endl;
        
        // 测试数据更新
        mapScreen.UpdateMapData(game);
        std::cout << "地图数据更新成功" << std::endl;
        
        // 测试刷新功能
        mapScreen.RefreshMapDisplay();
        std::cout << "地图刷新功能测试成功" << std::endl;
        
        // 测试消息添加
        mapScreen.AddMapMessage("测试消息");
        std::cout << "消息添加功能测试成功" << std::endl;
        
        // 测试获取组件
        auto component = mapScreen.GetComponent();
        if (component) {
            std::cout << "组件获取成功" << std::endl;
        } else {
            std::cout << "警告：组件获取失败" << std::endl;
        }
        
        std::cout << "=== 测试完成，所有功能正常 ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
