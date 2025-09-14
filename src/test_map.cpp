#include "core/map.h"
#include "player/player.h"
#include <iostream>
#include <memory>

int main() {
    std::cout << "=== 地图系统测试 ===" << std::endl;
    
    // 创建地图管理器
    MapManager mapManager;
    
    // 创建测试玩家
    Player player("测试玩家", 0, 0);
    
    // 显示初始地图
    std::cout << "\n初始地图:" << std::endl;
    auto mapLines = mapManager.renderMap(10, 8);
    for (const auto& line : mapLines) {
        std::cout << line << std::endl;
    }
    
    // 显示当前区块信息
    std::cout << "\n当前区块信息:" << std::endl;
    std::cout << mapManager.getCurrentBlockInfo() << std::endl;
    
    // 测试移动
    std::cout << "\n测试移动..." << std::endl;
    if (mapManager.movePlayer(1, 0)) {
        std::cout << "成功移动到 (1, 0)" << std::endl;
        std::cout << "当前区块信息:" << std::endl;
        std::cout << mapManager.getCurrentBlockInfo() << std::endl;
    }
    
    // 测试交互
    std::cout << "\n测试交互..." << std::endl;
    auto interactions = mapManager.getAvailableInteractions();
    std::cout << "可用交互: ";
    for (const auto& interaction : interactions) {
        switch (interaction) {
            case InteractionType::PICKUP: std::cout << "拾取 "; break;
            case InteractionType::BATTLE: std::cout << "战斗 "; break;
            case InteractionType::DIALOGUE: std::cout << "对话 "; break;
            case InteractionType::ACTIVATE: std::cout << "激活 "; break;
            case InteractionType::ENTER: std::cout << "进入 "; break;
        }
    }
    std::cout << std::endl;
    
    // 测试激活交互
    if (std::find(interactions.begin(), interactions.end(), InteractionType::ACTIVATE) != interactions.end()) {
        std::cout << "\n测试激活交互..." << std::endl;
        auto result = mapManager.interactWithCurrentBlock(player, InteractionType::ACTIVATE);
        std::cout << "交互结果: " << result.message << std::endl;
        std::cout << "成功: " << (result.success ? "是" : "否") << std::endl;
        std::cout << "区块完成: " << (result.blockCompleted ? "是" : "否") << std::endl;
    }
    
    // 显示最终地图
    std::cout << "\n最终地图:" << std::endl;
    mapLines = mapManager.renderMap(10, 8);
    for (const auto& line : mapLines) {
        std::cout << line << std::endl;
    }
    
    // 显示进度
    std::cout << "\n地图进度: " << mapManager.getCompletedBlocksCount() 
              << "/" << mapManager.getTotalBlocksCount() << std::endl;
    
    std::cout << "\n=== 测试完成 ===" << std::endl;
    return 0;
}
