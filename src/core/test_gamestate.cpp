#include "gamestate.h"
#include <iostream>
#include "../player/player.h"

void testGameState() {
    std::cout << "=== 测试游戏状态管理模块 ===" << std::endl;
    
    // 获取GameState实例
    GameState& gameState = GameState::getInstance();
    
    // 创建玩家
    auto player = std::make_shared<Player>("旅行者", 0, 0, 100);
    gameState.setPlayer(player);
    
    // 测试队伍管理
    std::cout << "\n1. 测试队伍管理:" << std::endl;
    TeamMember paimon("派蒙", 1, 50, 50, 5, 3, "风", "无");
    TeamMember amber("安柏", 20, 800, 800, 150, 60, "火", "弓");
    
    gameState.addTeamMember(paimon);
    gameState.addTeamMember(amber);
    
    const auto& team = gameState.getTeam();
    std::cout << "队伍成员:" << std::endl;
    for (const auto& member : team) {
        std::cout << "  - " << member.getStatusString() << std::endl;
    }
    
    // 测试背包管理
    std::cout << "\n2. 测试背包管理:" << std::endl;
    Item sword("铁剑", "一把普通的铁剑", ItemType::WEAPON, 100, 1, false, 15, 0, 0, 0);
    Item potion("生命药水", "恢复50点生命值", ItemType::CONSUMABLE, 50, 3, true, 0, 0, 0, 50);
    
    gameState.addToInventory(sword);
    gameState.addToInventory(potion);
    
    const auto& inventory = gameState.getInventory();
    std::cout << "背包物品:" << std::endl;
    for (size_t i = 0; i < inventory.size(); ++i) {
        std::cout << "  " << i + 1 << ". " << inventory[i].name 
                  << " x" << inventory[i].stackSize << std::endl;
    }
    
    // 测试位置管理
    std::cout << "\n3. 测试位置管理:" << std::endl;
    gameState.moveTo("蒙德城", "天使的馈赠", 120, 85, "蒙德城著名的酒馆");
    Position pos = gameState.getPosition();
    std::cout << "当前位置: " << pos.getFullPosition() << std::endl;
    std::cout << "描述: " << pos.description << std::endl;
    
    // 测试游戏时间
    std::cout << "\n4. 测试游戏时间:" << std::endl;
    std::cout << "当前游戏时间: " << gameState.getGameTimeString() << std::endl;
    
    // 移动并更新时间
    gameState.moveTo("低语森林", "林间小径", 45, 32, "宁静的森林小径");
    std::cout << "移动后时间: " << gameState.getGameTimeString() << std::endl;
    
    // 测试队伍成员状态更新
    std::cout << "\n5. 测试状态更新:" << std::endl;
    gameState.updateTeamMemberHealth("安柏", 650);
    TeamMember* updatedAmber = gameState.findTeamMember("安柏");
    if (updatedAmber) {
        std::cout << "安柏更新后状态: " << updatedAmber->getStatusString() << std::endl;
    }
    
    // 测试物品查找
    std::cout << "\n6. 测试物品查找:" << std::endl;
    Item* foundItem = gameState.findItemInInventory("生命药水");
    if (foundItem) {
        std::cout << "找到物品: " << foundItem->name 
                  << " x" << foundItem->stackSize << std::endl;
    }
    
    // 测试保存和加载功能
    std::cout << "\n7. 测试保存和加载功能:" << std::endl;
    
    // 保存游戏
    if (gameState.saveGame("test_save.json")) {
        std::cout << "游戏保存成功!" << std::endl;
        
        // 检查存档是否存在
        if (gameState.saveGameExists("test_save.json")) {
            std::cout << "存档文件存在" << std::endl;
        }
        
        // 创建新的GameState实例来测试加载
        GameState& newGameState = GameState::getInstance();
        
        // 重置一些状态以验证加载
        newGameState.removeTeamMember("派蒙");
        newGameState.removeTeamMember("安柏");
        
        // 加载游戏
        if (newGameState.loadGame("test_save.json")) {
            std::cout << "游戏加载成功!" << std::endl;
            
            // 验证队伍成员是否恢复
            const auto& loadedTeam = newGameState.getTeam();
            std::cout << "加载后的队伍成员:" << std::endl;
            for (const auto& member : loadedTeam) {
                std::cout << "  - " << member.getStatusString() << std::endl;
            }
            
            // 验证位置是否恢复
            Position loadedPos = newGameState.getPosition();
            std::cout << "加载后的位置: " << loadedPos.getFullPosition() << std::endl;
            
            // 验证游戏时间是否恢复
            std::cout << "加载后的游戏时间: " << newGameState.getGameTimeString() << std::endl;
        } else {
            std::cout << "游戏加载失败!" << std::endl;
        }
    } else {
        std::cout << "游戏保存失败!" << std::endl;
    }
    
    std::cout << "\n=== 测试完成 ===" << std::endl;
}

int main() {
    testGameState();
    return 0;
}