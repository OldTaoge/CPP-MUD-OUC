#include "core/map_v2.h"
#include "player/player.h"
#include <iostream>
#include <memory>

int main() {
    std::cout << "=== 新地图系统测试 ===" << std::endl;
    
    // 创建地图管理器
    MapManagerV2 mapManager;
    
    // 创建测试玩家
    Player player("测试玩家", 0, 0);
    
    // 显示当前区块信息
    std::cout << "\n当前区块信息:" << std::endl;
    std::cout << mapManager.getBlockInfo() << std::endl;
    
    // 显示当前地图
    std::cout << "\n当前区块地图:" << std::endl;
    auto mapLines = mapManager.renderCurrentBlock();
    for (const auto& line : mapLines) {
        std::cout << line << std::endl;
    }
    
    // 显示当前位置信息
    std::cout << "\n当前位置信息:" << std::endl;
    std::cout << mapManager.getCurrentCellInfo() << std::endl;
    
    // 测试移动
    std::cout << "\n测试移动..." << std::endl;
    if (mapManager.movePlayer(1, 0)) {
        std::cout << "成功向东移动" << std::endl;
        std::cout << "新位置信息:" << std::endl;
        std::cout << mapManager.getCurrentCellInfo() << std::endl;
    }
    
    // 测试交互
    std::cout << "\n测试交互..." << std::endl;
    auto interactions = mapManager.getAvailableInteractions();
    std::cout << "可用交互数量: " << interactions.size() << std::endl;
    for (const auto& interaction : interactions) {
        std::cout << "交互类型: ";
        switch (interaction) {
            case InteractionType::PICKUP: std::cout << "拾取"; break;
            case InteractionType::BATTLE: std::cout << "战斗"; break;
            case InteractionType::DIALOGUE: std::cout << "对话"; break;
            case InteractionType::ACTIVATE: std::cout << "激活"; break;
            case InteractionType::ENTER: std::cout << "进入"; break;
        }
        std::cout << std::endl;
    }
    
    // 测试区块切换
    std::cout << "\n测试区块切换..." << std::endl;
    if (mapManager.switchToBlock(1)) {
        std::cout << "成功切换到区块1" << std::endl;
        std::cout << "新区块信息:" << std::endl;
        std::cout << mapManager.getBlockInfo() << std::endl;
        
        // 显示新区块地图
        std::cout << "\n新区块地图:" << std::endl;
        auto newMapLines = mapManager.renderCurrentBlock();
        for (const auto& line : newMapLines) {
            std::cout << line << std::endl;
        }
    }
    
    // 显示进度
    std::cout << "\n地图进度: " << mapManager.getCompletedBlocksCount() 
              << "/" << mapManager.getTotalBlocksCount() << std::endl;
    
    std::cout << "\n=== 测试完成 ===" << std::endl;
    return 0;
}
