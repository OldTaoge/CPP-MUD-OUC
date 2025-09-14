#include "display/screens/gameplay.hpp"
#include "core/game.h"
#include <iostream>
#include <memory>

int main() {
    std::cout << "=== 工具菜单测试 ===" << std::endl;
    
    try {
        // 创建游戏对象
        Game game;
        game.StartNewGame();
        
        // 创建游戏界面
        GameplayScreen gameplayScreen(&game);
        
        std::cout << "游戏界面创建成功" << std::endl;
        
        // 测试工具菜单显示
        gameplayScreen.ShowToolOverlay();
        std::cout << "工具菜单显示成功" << std::endl;
        
        // 测试工具菜单隐藏
        gameplayScreen.HideToolOverlay();
        std::cout << "工具菜单隐藏成功" << std::endl;
        
        // 测试游戏命令
        gameplayScreen.HandleGameCommand("move north");
        std::cout << "游戏命令处理成功" << std::endl;
        
        std::cout << "=== 测试完成 ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
