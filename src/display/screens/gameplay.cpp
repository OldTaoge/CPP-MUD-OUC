//
// Created by Assistant on 2025/1/1.
//

#include "gameplay.hpp"
#include "../player/player.h"
#include "../core/map.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/event.hpp>
#include <algorithm>
#include <iostream>

using namespace ftxui;

GameplayScreen::GameplayScreen(Player* player) : player_(player), mapManager_(nullptr) {
    // 初始化工具选项
    tool_options_ = {
        "背包 - 查看和管理物品",
        "地图 - 查看世界地图和传送点", 
        "任务 - 查看当前任务和成就",
        "设置 - 调整游戏设置",
        "返回 - 保存游戏或退出"
    };
    
    // 初始化命令选项
    command_options_ = {
        "移动 - 探索周围环境",
        "攻击 - 进入战斗状态",
        "查看 - 仔细观察环境",
        "交谈 - 与NPC对话",
        "收集 - 拾取物品",
        "帮助 - 查看可用命令"
    };
    
    // 使用传入的玩家对象初始化游戏状态
    UpdatePlayerInfo(*player_);
    player_max_hp_ = 100; // 假设最大生命值为100
    player_status_ = "正常";
    team_members_ = {"派蒙", "温迪", "钟离"};
    
    // 添加一些初始消息
    AddChatMessage("欢迎来到《原神》世界！我是派蒙，你的向导。", true);
    AddChatMessage("有什么需要帮助的吗？输入帮助查看命令。");
    UpdateGameStatus("你站在蒙德城的广场上，周围是熙熙攘攘的人群。");
    
    // 初始化地图实体列表
    UpdateMapEntities();
    
    // 创建聊天输入组件
    chat_input_ = Input(&chat_input_buffer_, "与派蒙对话...");
    
    // 创建单一工具按钮
    tool_button_ = Button("工具", [this] {
        this->ShowToolOverlay();
    });
    
    // 创建工具选项按钮
    tool_option_buttons_.clear();
    for (size_t i = 0; i < tool_options_.size(); ++i) {
        auto button = Button(tool_options_[i], [this, i] {
            this->HandleToolOption(i);
        });
        tool_option_buttons_.push_back(button);
    }
    
    // 创建关闭按钮
    close_button_ = Button("关闭", [this] {
        this->HideToolOverlay();
    });
    
    // 创建工具叠加图层
    tool_overlay_ = Container::Vertical({});
    for (auto& button : tool_option_buttons_) {
        tool_overlay_->Add(button);
    }
    tool_overlay_->Add(close_button_);
    
    // 创建主组件
    component_ = Container::Vertical({
        chat_input_,
        tool_button_,
        tool_overlay_
    });
    
    // 添加键盘事件处理
    component_ = CatchEvent(component_, [this](Event event) {
        if (show_map_overlay_) {
            if (event.is_character()) {
                if (event.character() == "q" || event.character() == "Q") {
                    HideMapOverlay();
                    return true;
                }
            } else if (event.is_mouse()) {
                // 处理鼠标事件
                return false;
            } else if (event == Event::ArrowUp) {
                if (selected_map_entity_ > 0) {
                    selected_map_entity_--;
                }
                return true;
            } else if (event == Event::ArrowDown) {
                if (selected_map_entity_ < current_map_entities_.size() - 1) {
                    selected_map_entity_++;
                }
                return true;
            } else if (event == Event::Return) {
                HandleMapEntitySelection(selected_map_entity_);
                return true;
            } else if (event == Event::Escape) {
                HideMapOverlay();
                return true;
            }
        } else if (!show_tool_overlay_) {
            // 环境交互模式 - 使用e键进入
            if (event.is_character() && (event.character() == "e" || event.character() == "E")) {
                if (!current_map_entities_.empty()) {
                    AddChatMessage("派蒙: 按方向键选择对象，按Enter键交互，按ESC退出交互模式。", true);
                    // 模拟选中第一个可交互对象
                    selected_map_entity_ = 0;
                    // 立即执行交互
                    HandleMapEntitySelection(selected_map_entity_);
                } else {
                    AddChatMessage("派蒙: 当前区域没有可交互的对象。", true);
                }
                return true;
            }
            
            // 命令选项选择逻辑
            if (event == Event::ArrowUp) {
                if (selected_command_option_ > 0) {
                    selected_command_option_--;
                }
                return true;
            } else if (event == Event::ArrowDown) {
                if (selected_command_option_ < command_options_.size() - 1) {
                    selected_command_option_++;
                }
                return true;
            } else if (event == Event::Return) {
                // 执行选中的命令
                HandleCommandOption(selected_command_option_);
                return true;
            }
        }
        return false;
    });
    
    // 设置渲染器
    component_ = Renderer(component_, [this] {
        // 检查输入缓冲区变化并处理
        static std::string last_chat_input = "";
        
        if (chat_input_buffer_ != last_chat_input && !chat_input_buffer_.empty() && 
            chat_input_buffer_.find('\n') != std::string::npos) {
            // 聊天输入完成
            std::string input = chat_input_buffer_;
            input.erase(std::remove(input.begin(), input.end(), '\n'), input.end());
            if (!input.empty()) {
                AddChatMessage("你: " + input);
                AddChatMessage("派蒙: 我明白了！让我想想...", true);
            }
            chat_input_buffer_.clear();
        }
        
        last_chat_input = chat_input_buffer_;
        
        // 构建聊天消息元素
        Elements chat_elements;
        for (const auto& msg : chat_messages_) {
            chat_elements.push_back(text(msg) | color(Color::White));
        }
        
        // 构建游戏消息元素
        Elements game_elements;
        for (const auto& msg : game_messages_) {
            game_elements.push_back(text(msg) | color(Color::White));
        }
        
        // 构建队伍成员元素
        Elements team_elements;
        for (const auto& member : team_members_) {
            team_elements.push_back(text("• " + member) | color(Color::Blue));
        }
        
        // 顶部：LLM对话框 + 右侧工具按钮
        auto chat_area = vbox({
            hbox({
                text("派蒙") | bold | color(Color::Yellow),
                text("《原神》MUD版") | bold | color(Color::Cyan) | hcenter | flex,
                tool_button_->Render()
            }),
            separator(),
            vbox({
                vbox(chat_elements) | flex | border,
                hbox({
                    text("你: ") | color(Color::Cyan),
                    chat_input_->Render() | flex
                })
            }) | flex
        }) | size(HEIGHT, EQUAL, 15);  // 固定高度为15行
        /*
        auto tool_area = vbox({
            text("工具") | bold | color(Color::Blue),
            separator(),
            hbox({
                text("工具") | bold | color(Color::Blue),
                tool_button_->Render() | hcenter
            }) | hcenter
        }) | size(WIDTH, LESS_THAN, 25);
        */
        auto top_row = hbox({
            chat_area | flex,
            separator(),
           // tool_area
        });
        
        // 中间：游戏交互主界面
        auto game_area = vbox({
            text("游戏世界") | bold | color(Color::Green),
            separator(),
            vbox({
                vbox(game_elements) | flex | border,
                
                // 当前区域可交互对象
                vbox({
                    text("周围环境:") | color(Color::Green),
                    separator(),
                    vbox(
                        [this]() {
                            Elements interactables;
                            if (current_map_entities_.empty()) {
                                interactables.push_back(text("当前区域没有可交互的对象。"));
                            } else {
                                for (const auto& entity : current_map_entities_) {
                                    // 根据实体类型设置不同颜色
                                    if (entity.find("NPC:") != std::string::npos) {
                                        interactables.push_back(text("  " + entity) | color(Color::Cyan));
                                    } else if (entity.find("敌人:") != std::string::npos) {
                                        interactables.push_back(text("  " + entity) | color(Color::Red));
                                    } else if (entity.find("物品:") != std::string::npos) {
                                        interactables.push_back(text("  " + entity) | color(Color::Yellow));
                                    } else {
                                        interactables.push_back(text("  " + entity));
                                    }
                                }
                            }
                            return interactables;
                        }()
                    ) | border
                }),
                
                // 游戏命令选项
                vbox({
                    text("请选择命令 (↑↓箭头选择, Enter确认):") | color(Color::Red),
                    separator(),
                    vbox(
                        [this]() {
                            Elements options;
                            for (size_t i = 0; i < command_options_.size(); ++i) {
                                if (i == selected_command_option_) {
                                    options.push_back(
                                        text("> " + command_options_[i])
                                        | color(Color::Yellow)
                                        | bgcolor(Color::Blue)
                                    );
                                } else {
                                    options.push_back(text("  " + command_options_[i]));
                                }
                            }
                            return options;
                        }()
                    ) | border
                })
            }) | flex
        }) | size(HEIGHT, EQUAL, 20);  // 固定高度为20行
        
        // 底部：人物HP + 人物状态 + 队伍状态
        auto status_area = hbox({
            vbox({
                text("生命值") | bold | color(Color::Red),
                text(player_name_ + ": " + std::to_string(player_hp_) + "/" + std::to_string(player_max_hp_)) | color(Color::Red)
            }) | border | flex,
            separator(),
            vbox({
                text("状态") | bold | color(Color::Yellow),
                text(player_status_) | color(Color::Yellow)
            }) | border | flex,
            separator(),
            vbox({
                text("队伍") | bold | color(Color::Blue),
                vbox(team_elements)
            }) | border | flex
        }) | size(HEIGHT, EQUAL, 8);  // 固定高度为8行
        
        // 基础界面
        auto base_interface = vbox({
            top_row | flex,
            separator(),
            game_area | flex,
            separator(),
            status_area
        }) | border;
        
        // 地图叠加图层
        if (show_map_overlay_) {
            Elements map_elements;
            
            // 显示当前区域信息
            if (mapManager_) {
                MapArea* currentArea = mapManager_->getArea(mapManager_->getCurrentArea());
                if (currentArea) {
                    map_elements.push_back(text("当前区域: " + currentArea->name) | color(Color::Cyan) | bold);
                    map_elements.push_back(text(currentArea->description) | color(Color::White));
                    map_elements.push_back(separator());
                }
            }
            
            // 显示可交互实体列表
            map_elements.push_back(text("可交互内容:") | color(Color::Green) | bold);
            for (size_t i = 0; i < current_map_entities_.size(); ++i) {
                if (i == selected_map_entity_) {
                    map_elements.push_back(text("> " + current_map_entities_[i]) | color(Color::Yellow) | bgcolor(Color::Blue));
                } else {
                    map_elements.push_back(text("  " + current_map_entities_[i]));
                }
            }
            
            // 添加操作说明
            map_elements.push_back(separator());
            map_elements.push_back(text("操作说明:") | color(Color::Blue) | bold);
            map_elements.push_back(text("方向键 - 选择交互对象"));
            map_elements.push_back(text("Enter - 确认交互"));
            map_elements.push_back(text("ESC - 返回"));
            
            auto map_overlay_element = vbox({
                text("地图交互") | bold | color(Color::Green) | hcenter,
                separator(),
                vbox(map_elements) | border,
                separator(),
                text("按ESC返回游戏") | color(Color::Yellow) | hcenter
            }) | border | bgcolor(Color::DarkGreen) | color(Color::White);
            
            return map_overlay_element | hcenter | vcenter;
        }
        // 工具叠加图层
        else if (show_tool_overlay_) {
            Elements overlay_options;
            for (auto& button : tool_option_buttons_) {
                overlay_options.push_back(button->Render());
            }
            
            auto overlay_element = vbox({
                text("工具菜单") | bold | color(Color::Magenta) | hcenter,
                separator(),
                vbox(overlay_options) | border,
                separator(),
                close_button_->Render() | hcenter
            }) | border | bgcolor(Color::DarkBlue) | color(Color::White);
            
            return overlay_element | hcenter | vcenter;
        } else {
            return base_interface;
        }
    });
}

Component GameplayScreen::GetComponent() {
    return component_;
}

void GameplayScreen::UpdatePlayerInfo(const Player& player) {
    player_name_ = player.name;
    player_hp_ = player.health;
    // 这里可以添加更多玩家信息的更新
    
    // 定期更新游戏状态，确保显示最新的玩家信息
    UpdateGameStatus("你站在蒙德城的广场上，周围是熙熙攘攘的人群。");
}

void GameplayScreen::AddChatMessage(const std::string& message, bool isLLM) {
    chat_messages_.push_back(message);
    // 限制消息数量，避免内存占用过多
    if (chat_messages_.size() > 50) {
        chat_messages_.erase(chat_messages_.begin());
    }
}

void GameplayScreen::UpdateGameStatus(const std::string& status) {
    game_messages_.clear();
    game_messages_.push_back(status);
}

void GameplayScreen::UpdateTeamStatus(const std::vector<std::string>& teamMembers) {
    team_members_ = teamMembers;
}

void GameplayScreen::HandleToolButton(int buttonIndex) {
    switch (buttonIndex) {
        case 0: // 背包
            AddChatMessage("派蒙: 打开背包查看物品", true);
            break;
        case 1: // 地图
            AddChatMessage("派蒙: 显示世界地图", true);
            ShowMapOverlay();
            break;
        case 2: // 任务
            AddChatMessage("派蒙: 查看当前任务", true);
            break;
        case 3: // 设置
            if (navigation_callback_) {
                navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Settings"));
            }
            break;
        case 4: // 返回
            if (navigation_callback_) {
                navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "MainMenu"));
            }
            break;
    }
}

// 命令选项处理函数
void GameplayScreen::HandleCommandOption(int optionIndex) {
    if (optionIndex < 0 || optionIndex >= command_options_.size()) {
        return;
    }
    
    switch (optionIndex) {
        case 0: // 移动
            UpdateGameStatus("你开始移动...");
            AddChatMessage("派蒙: 好的，我们走吧！", true);
            break;
        case 1: // 攻击
            UpdateGameStatus("你进入战斗状态！");
            AddChatMessage("派蒙: 小心！敌人出现了！", true);
            break;
        case 2: // 查看
            UpdateGameStatus("你仔细观察周围的环境...");
            AddChatMessage("派蒙: 这里有很多有趣的东西呢！", true);
            break;
        case 3: // 交谈
            UpdateGameStatus("你想与谁交谈？");
            AddChatMessage("派蒙: 先找到NPC，然后尝试与他们对话吧！", true);
            break;
        case 4: // 收集
            UpdateGameStatus("你开始收集物品...");
            AddChatMessage("派蒙: 周围有没有什么可以收集的东西呢？", true);
            break;
        case 5: // 帮助
            AddChatMessage("派蒙: 使用↑↓箭头键选择命令，按Enter键执行。", true);
            AddChatMessage("可用命令包括：移动、攻击、查看、交谈、收集和帮助。", true);
            break;
    }
}

// 保留原有的命令处理函数以兼容可能的其他调用
void GameplayScreen::HandleGameCommand(const std::string& command) {
    std::string lowerCommand = command;
    std::transform(lowerCommand.begin(), lowerCommand.end(), lowerCommand.begin(), ::tolower);
    
    if (lowerCommand.find("移动") != std::string::npos || lowerCommand.find("走") != std::string::npos) {
        HandleCommandOption(0);  // 调用移动命令
    } else if (lowerCommand.find("攻击") != std::string::npos || lowerCommand.find("战斗") != std::string::npos) {
        HandleCommandOption(1);  // 调用攻击命令
    } else if (lowerCommand.find("查看") != std::string::npos || lowerCommand.find("观察") != std::string::npos) {
        HandleCommandOption(2);  // 调用查看命令
    } else if (lowerCommand.find("交谈") != std::string::npos || lowerCommand.find("对话") != std::string::npos) {
        HandleCommandOption(3);  // 调用交谈命令
    } else if (lowerCommand.find("收集") != std::string::npos || lowerCommand.find("拾取") != std::string::npos) {
        HandleCommandOption(4);  // 调用收集命令
    } else if (lowerCommand.find("帮助") != std::string::npos) {
        HandleCommandOption(5);  // 调用帮助命令
    } else {
        UpdateGameStatus("你尝试了 '" + command + "'");
        AddChatMessage("派蒙: 我不太明白你的意思，试试其他命令吧！", true);
    }
}

void GameplayScreen::ShowToolOverlay() {
    show_tool_overlay_ = true;
}

void GameplayScreen::HideToolOverlay() {
    show_tool_overlay_ = false;
}

void GameplayScreen::HandleToolOption(int optionIndex) {
    HideToolOverlay();
    
    switch (optionIndex) {
        case 0: // 背包
            AddChatMessage("派蒙: 打开背包查看物品", true);
            if (navigation_callback_) {
                navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Inventory"));
            }
            break;
        case 1: // 地图
            AddChatMessage("派蒙: 显示世界地图", true);
            ShowMapOverlay();
            break;
        case 2: // 任务
            AddChatMessage("派蒙: 查看当前任务", true);
            if (navigation_callback_) {
                navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Quest"));
            }
            break;
        case 3: // 设置
            if (navigation_callback_) {
                navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Settings"));
            }
            break;
        case 4: // 返回
            if (navigation_callback_) {
                navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "MainMenu"));
            }
            break;
    }
}

// 地图相关方法实现
void GameplayScreen::ShowMapOverlay() {
    show_map_overlay_ = true;
    show_tool_overlay_ = false;
    UpdateMapEntities();
}

void GameplayScreen::HideMapOverlay() {
    show_map_overlay_ = false;
    selected_map_entity_ = 0;
}

void GameplayScreen::HandleMapEntitySelection(int entityIndex) {
    if (entityIndex < 0 || entityIndex >= current_map_entities_.size()) {
        return;
    }
    
    if (!mapManager_) {
        AddChatMessage("派蒙: 地图系统未加载", true);
        return;
    }
    
    MapArea* currentArea = mapManager_->getArea(mapManager_->getCurrentArea());
    if (!currentArea) {
        AddChatMessage("派蒙: 当前区域无效", true);
        return;
    }
    
    std::string entityName = current_map_entities_[entityIndex];
    
    if (entityName.find("NPC:") != std::string::npos) {
        // 处理NPC交互
        std::string npcName = entityName.substr(4); // 移除"NPC: "前缀
        auto npcs = currentArea->getNPCs();
        for (auto npc : npcs) {
            if (npc->name == npcName) {
                AddChatMessage("派蒙: 与 " + npcName + " 对话", true);
                AddChatMessage(npcName + ": " + npc->getRandomDialogue());
                break;
            }
        }
    } else if (entityName.find("敌人:") != std::string::npos) {
        // 处理敌人交互
        std::string enemyName = entityName.substr(3); // 移除"敌人: "前缀
        auto enemies = currentArea->getEnemies();
        for (auto enemy : enemies) {
            if (enemy->name == enemyName) {
                AddChatMessage("派蒙: 小心！发现了 " + enemyName, true);
                AddChatMessage("敌人信息: 生命值 " + std::to_string(enemy->health) + 
                             ", 攻击力 " + std::to_string(enemy->attack) + 
                             ", 防御力 " + std::to_string(enemy->defense));
                break;
            }
        }
    } else if (entityName.find("物品:") != std::string::npos) {
        // 处理物品交互
        std::string itemName = entityName.substr(3); // 移除"物品: "前缀
        auto collectibles = currentArea->getCollectibles();
        for (auto collectible : collectibles) {
            if (collectible->name == itemName) {
                if (collectible->canCollect()) {
                    Item collectedItem = collectible->collect();
                    AddChatMessage("派蒙: 你拾取了 " + collectedItem.name + "!", true);
                    AddChatMessage("物品描述: " + collectedItem.description);
                    mapManager_->removeCollectibleFromCurrentArea(collectible);
                    UpdateMapEntities(); // 更新地图实体列表
                } else {
                    AddChatMessage("派蒙: " + itemName + " 已经被拾取了", true);
                }
                break;
            }
        }
    }
    
    HideMapOverlay();
}

void GameplayScreen::UpdateMapEntities() {
    // 保存更新前的实体数量
    size_t old_entity_count = current_map_entities_.size();
    
    current_map_entities_.clear();
    
    if (!mapManager_) {
        return;
    }
    
    MapArea* currentArea = mapManager_->getArea(mapManager_->getCurrentArea());
    if (currentArea) {
        current_map_entities_ = currentArea->getInteractableList();
    }
    
    // 当实体列表更新时，重置选中项
    selected_map_entity_ = 0;
    
    // 如果进入了新区域并且有可交互对象，显示提示
    if (old_entity_count == 0 && !current_map_entities_.empty()) {
        AddChatMessage("派蒙: 这个区域有可交互的对象，按'e'键可以与它们互动。", true);
    }
}

void GameplayScreen::SetMapManager(MapManager* mapManager) {
    mapManager_ = mapManager;
    UpdateMapEntities();
}
