// =============================================
// 文件: game.cpp
// 描述: 游戏核心实现。负责新游戏初始化、存档读写、地图切换与交互、
//       玩家经验与升级流程，以及向 UI 层暴露必要的查询/操作接口。
// 注意: 避免在此文件中引入与 UI 强耦合的头，以保持核心层纯净。
// =============================================
#include "game.h"
#include <iostream>

// 构造函数：初始化玩家与状态。地图与队伍的完整初始化在 StartNewGame 中完成。
Game::Game() 
    : player_("默认玩家", 1, 1), 
      currentState_(GameState::MAIN_MENU) {
    // 预留：保存系统的初始化如需异步或检查磁盘，可在此扩展
}

// 开始新游戏：重置玩家、队伍、背包，并将地图定位到默认区块
void Game::StartNewGame() {
    std::cout << "开始新游戏..." << std::endl;
    InitializeNewPlayer();
    currentState_ = GameState::PLAYING;
    std::cout << "新游戏开始！" << std::endl;
}

// 加载游戏：采用带地图状态的加载流程
void Game::LoadGame() {
    LoadGameWithMapState();
}

// 加载指定存档文件
void Game::LoadGame(const std::string& saveFileName) {
    std::cout << "正在加载游戏: " << saveFileName << std::endl;
    
    int currentBlockId = 0;
    SaveResult result = gameSave_.loadGame(player_, currentBlockId, saveFileName);
    
    if (result == SaveResult::SUCCESS) {
        std::cout << "游戏加载成功！" << std::endl;
        
        // 初始化地图系统到保存的区块
        mapManager_.switchToBlock(currentBlockId, player_.x, player_.y);
        
        currentState_ = GameState::PLAYING;
        std::cout << "地图状态已恢复到区块 " << currentBlockId << "，位置 (" << player_.x << ", " << player_.y << ")" << std::endl;
    } else {
        std::cout << "游戏加载失败，错误代码: " << static_cast<int>(result) << std::endl;
    }
}

// 加载游戏并恢复地图状态：若无存档则启动新游戏
void Game::LoadGameWithMapState() {
    std::cout << "正在加载游戏..." << std::endl;
    
    // 检查是否有存档文件
    auto saveFiles = gameSave_.listSaveFiles();
    if (saveFiles.empty()) {
        std::cout << "没有找到存档文件，将开始新游戏。" << std::endl;
        StartNewGame();
        return;
    }
    
    // 使用第一个存档文件（可以后续扩展为让用户选择）
    std::string saveFile = saveFiles[0];
    int currentBlockId = 0;
    SaveResult result = gameSave_.loadGame(player_, currentBlockId, saveFile);
    
    if (result == SaveResult::SUCCESS) {
        std::cout << "游戏加载成功！" << std::endl;
        
        // 初始化地图系统到保存的区块
        mapManager_.switchToBlock(currentBlockId, player_.x, player_.y);
        
        currentState_ = GameState::PLAYING;
        std::cout << "地图状态已恢复到区块 " << currentBlockId << "，位置 (" << player_.x << ", " << player_.y << ")" << std::endl;
    } else {
        std::cout << "游戏加载失败，错误代码: " << static_cast<int>(result) << std::endl;
        std::cout << "将开始新游戏。" << std::endl;
        StartNewGame();
    }
}

// 保存游戏（带地图信息）
void Game::SaveGame() {
    SaveGameWithMapState();
}

// 保存到指定存档文件名
void Game::SaveGame(const std::string& saveFileName) {
    std::cout << "正在保存游戏: " << saveFileName << std::endl;
    
    // 获取当前地图状态
    int currentBlockId = mapManager_.getCurrentBlockId();
    std::cout << "当前区块ID: " << currentBlockId << std::endl;
    std::cout << "玩家位置: (" << player_.x << ", " << player_.y << ")" << std::endl;
    
    SaveResult result = gameSave_.saveGame(player_, currentBlockId, saveFileName);
    if (result == SaveResult::SUCCESS) {
        std::cout << "游戏保存成功！当前区块: " << currentBlockId << std::endl;
    } else {
        std::cout << "游戏保存失败，错误代码: " << static_cast<int>(result) << std::endl;
    }
}

// 自动保存到默认文件名（示例：autosave.json）
void Game::SaveGameWithMapState() {
    std::cout << "正在保存游戏..." << std::endl;
    
    // 获取当前地图状态
    int currentBlockId = mapManager_.getCurrentBlockId();
    
    SaveResult result = gameSave_.saveGame(player_, currentBlockId, "autosave.json");
    if (result == SaveResult::SUCCESS) {
        std::cout << "游戏保存成功！当前区块: " << currentBlockId << std::endl;
    } else {
        std::cout << "游戏保存失败，错误代码: " << static_cast<int>(result) << std::endl;
    }
}

// 初始化新玩家、队伍与背包，并与地图管理器同步位置
void Game::InitializeNewPlayer() {
    // 重置玩家状态
    player_ = Player("玩家", 0, 0);
    
    // 设置初始队伍
    setupInitialTeam();
    
    // 设置初始背包
    setupInitialInventory();
    
    // 初始化地图系统 - 从第一个区块开始
    std::cout << "初始化地图系统..." << std::endl;
    bool switchResult = mapManager_.switchToBlock(0, 4, 4);
    std::cout << "地图切换结果: " << (switchResult ? "成功" : "失败") << std::endl;
    std::cout << "当前区块ID: " << mapManager_.getCurrentBlockId() << std::endl;
    
    // 同步玩家位置到地图管理器
    auto pos = mapManager_.getPlayerPosition();
    player_.x = pos.first;
    player_.y = pos.second;
    std::cout << "玩家位置同步: (" << player_.x << ", " << player_.y << ")" << std::endl;
    
    std::cout << "新游戏初始化完成！" << std::endl;
}

// 列出存档文件
std::vector<std::string> Game::getSaveFiles() const {
    return gameSave_.listSaveFiles();
}

// 读取存档的概要信息（不加载完整游戏）
GameSave::SaveInfo Game::getSaveInfo(const std::string& saveFileName) const {
    return gameSave_.getSaveInfo(saveFileName);
}

// 检查指定名称的存档是否存在
bool Game::saveExists(const std::string& saveFileName) const {
    return gameSave_.saveExists(saveFileName);
}

// 删除指定名称的存档
bool Game::deleteSave(const std::string& saveFileName) const {
    return gameSave_.deleteSave(saveFileName);
}

// 直接更新玩家坐标。若涉及地图规则与碰撞，请使用 movePlayer。
void Game::updatePlayerPosition(int x, int y) {
    player_.x = x;
    player_.y = y;
}

// 增加经验并在达到阈值时自动升级
void Game::addExperience(int exp) {
    player_.experience += exp;
    
    // 检查是否升级
    int requiredExp = player_.level * 100; // 简单的升级公式
    if (player_.experience >= requiredExp) {
        levelUp();
    }
}

// 升级逻辑：提升玩家等级，重置经验，并同步队伍成员
void Game::levelUp() {
    player_.level++;
    player_.experience = 0; // 重置经验值
    
    // 提升队伍成员等级
    for (auto& member : player_.teamMembers) {
        member->setLevel(member->getLevel() + 1);
        member->resetHealth(); // 升级时恢复生命值
    }
    
    std::cout << "恭喜！玩家升级到 " << player_.level << " 级！" << std::endl;
}

// 初始化背包：可按需添加初始物品（示例保留注释以示用法）
void Game::setupInitialInventory() {
    // 添加一些初始物品
    //player_.addItemToInventory(ItemFactory::createWeapon("新手剑", WeaponType::ONE_HANDED_SWORD, Rarity::ONE_STAR));
    // player_.addItemToInventory(ItemFactory::createWeapon("精钢剑", WeaponType::ONE_HANDED_SWORD, Rarity::THREE_STAR));
    // player_.addItemToInventory(ItemFactory::createArtifact("冒险家尾羽", ArtifactType::PLUME_OF_DEATH, Rarity::THREE_STAR));
    // player_.addItemToInventory(ItemFactory::createFood("苹果", FoodType::RECOVERY, Rarity::ONE_STAR));
    // player_.addItemToInventory(ItemFactory::createFood("辣椒", FoodType::ATTACK, Rarity::TWO_STAR));
    // player_.addItemToInventory(ItemFactory::createMaterial("史莱姆凝液", MaterialType::MONSTER_DROP, Rarity::ONE_STAR));
    // player_.addItemToInventory(ItemFactory::createMaterial("胡萝卜", MaterialType::COOKING_INGREDIENT, Rarity::ONE_STAR));
}

// 初始化队伍：为首位成员装备基础武器并设为活跃
void Game::setupInitialTeam() {
    // Player构造函数已经添加了"旅行者"作为初始成员
    // 这里为初始成员装备武器和设置状态
    
    if (!player_.teamMembers.empty()) {
        // 为第一个队伍成员装备初始武器
        auto sword = ItemFactory::createWeapon("新手剑", WeaponType::ONE_HANDED_SWORD, Rarity::ONE_STAR);
        if (sword) {
            auto weapon = std::dynamic_pointer_cast<Weapon>(sword);
            if (weapon) {
                player_.teamMembers[0]->equipWeapon(weapon);
            }
        }
        
        // 确保第一个成员是活跃状态
        player_.teamMembers[0]->setStatus(MemberStatus::ACTIVE);
        player_.activeMember = player_.teamMembers[0];
    }
}

// 地图系统相关方法 ---------------------------------------------------------
InteractionResult Game::interactWithMap(InteractionType interactionType) {
    return mapManager_.interactWithCurrentCell(player_, interactionType);
}

// 请求移动玩家：若地图允许移动，则同步玩家坐标
bool Game::movePlayer(int deltaX, int deltaY) {
    if (mapManager_.movePlayer(deltaX, deltaY)) {
        // 更新玩家坐标
        auto pos = mapManager_.getPlayerPosition();
        player_.x = pos.first;
        player_.y = pos.second;
        return true;
    }
    return false;
}

// 获取当前位置可用的交互选项
std::vector<InteractionType> Game::getAvailableMapInteractions() const {
    return mapManager_.getAvailableInteractions();
}
