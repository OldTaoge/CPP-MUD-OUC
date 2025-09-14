#include "mapScreen.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/dom/table.hpp>
#include <sstream>

using namespace ftxui;

MapScreen::MapScreen(MapManager* mapManager) 
    : mapManager_(mapManager), selectedArea_(0), selectedEntity_(0), currentMessage_("") {
    
    mapOverview_ = CreateMapOverview();
    areaDetails_ = CreateAreaDetails();
    interactionPanel_ = CreateInteractionPanel();
    navigationPanel_ = CreateNavigationPanel();
    
    UpdateDisplay();
}

Component MapScreen::GetComponent() {
    auto container = Container::Vertical({
        mapOverview_,
        areaDetails_,
        interactionPanel_,
        navigationPanel_
    });
    
    return container;
}

Component MapScreen::CreateMapOverview() {
    auto overview = Container::Vertical({});
    
    auto overviewComponent = Renderer(overview, [this]() {
        std::string overviewText = mapManager_ ? mapManager_->getMapOverview() : "地图未加载";
        
        std::vector<Element> elements;
        std::istringstream iss(overviewText);
        std::string line;
        
        while (std::getline(iss, line)) {
            if (line.find("[当前位置]") != std::string::npos) {
                elements.push_back(text(line) | color(Color::Green) | bold);
            } else if (line.find("===") != std::string::npos) {
                elements.push_back(text(line) | color(Color::Yellow) | bold);
            } else if (line.find(".") == 0) {
                elements.push_back(text(line) | color(Color::Blue));
            } else {
                elements.push_back(text(line));
            }
        }
        
        return vbox(std::move(elements));
    });
    
    return overviewComponent;
}

Component MapScreen::CreateAreaDetails() {
    auto areaDetails = Container::Vertical({});
    
    auto areaDetailsComponent = Renderer(areaDetails, [this]() {
        if (!mapManager_) {
            return text("地图管理器未加载");
        }
        
        MapArea* currentArea = mapManager_->getArea(mapManager_->getCurrentArea());
        if (!currentArea) {
            return text("当前区域无效");
        }
        
        std::string areaInfo = currentArea->getAreaInfo();
        std::vector<Element> elements;
        std::istringstream iss(areaInfo);
        std::string line;
        
        while (std::getline(iss, line)) {
            if (line.find("区域:") != std::string::npos) {
                elements.push_back(text(line) | color(Color::Cyan) | bold);
            } else if (line.find("描述:") != std::string::npos) {
                elements.push_back(text(line) | color(Color::White));
            } else if (line.find("数量:") != std::string::npos) {
                elements.push_back(text(line) | color(Color::Yellow));
            } else {
                elements.push_back(text(line));
            }
        }
        
        return vbox(std::move(elements));
    });
    
    return areaDetailsComponent;
}

Component MapScreen::CreateInteractionPanel() {
    auto interactionPanel = Container::Vertical({});
    
    auto interactionComponent = Renderer(interactionPanel, [this]() {
        if (!mapManager_) {
            return text("地图管理器未加载");
        }
        
        MapArea* currentArea = mapManager_->getArea(mapManager_->getCurrentArea());
        if (!currentArea) {
            return text("当前区域无效");
        }
        
        currentInteractables_ = currentArea->getInteractableList();
        
        if (currentInteractables_.empty()) {
            return text("当前区域没有可交互的内容");
        }
        
        std::vector<Element> elements;
        elements.push_back(text("可交互内容:") | color(Color::Green) | bold);
        
        for (size_t i = 0; i < currentInteractables_.size(); ++i) {
            std::string prefix = std::to_string(i + 1) + ". ";
            if (i == selectedEntity_) {
                elements.push_back(text(prefix + currentInteractables_[i]) | color(Color::Yellow) | bgcolor(Color::Blue));
            } else {
                elements.push_back(text(prefix + currentInteractables_[i]));
            }
        }
        
        if (!currentMessage_.empty()) {
            elements.push_back(separator());
            elements.push_back(text("消息: " + currentMessage_) | color(Color::Magenta));
        }
        
        return vbox(std::move(elements));
    });
    
    return interactionComponent;
}

Component MapScreen::CreateNavigationPanel() {
    auto navigationPanel = Container::Vertical({});
    
    auto navigationComponent = Renderer(navigationPanel, [this]() {
        std::vector<Element> elements;
        elements.push_back(text("导航控制:") | color(Color::Blue) | bold);
        elements.push_back(text("WASD - 选择区域"));
        elements.push_back(text("Enter - 确认选择"));
        elements.push_back(text("方向键 - 选择交互对象"));
        elements.push_back(text("ESC - 返回主菜单"));
        
        return vbox(std::move(elements));
    });
    
    return navigationComponent;
}

void MapScreen::HandleAreaSelection(int areaIndex) {
    if (mapManager_ && mapManager_->canMoveToArea(areaIndex)) {
        mapManager_->moveToArea(areaIndex);
        selectedEntity_ = 0;
        currentMessage_ = "已移动到 " + mapManager_->getArea(areaIndex)->name;
        UpdateDisplay();
    }
}

void MapScreen::HandleInteraction(int entityIndex) {
    if (!mapManager_ || entityIndex < 0 || entityIndex >= currentInteractables_.size()) {
        return;
    }
    
    MapArea* currentArea = mapManager_->getArea(mapManager_->getCurrentArea());
    if (!currentArea) {
        return;
    }
    
    std::string entityName = currentInteractables_[entityIndex];
    
    if (entityName.find("NPC:") != std::string::npos) {
        // 处理NPC交互
        std::string npcName = entityName.substr(4); // 移除"NPC: "前缀
        auto npcs = currentArea->getNPCs();
        for (auto npc : npcs) {
            if (npc->name == npcName) {
                currentMessage_ = "与 " + npcName + " 对话: " + npc->getRandomDialogue();
                break;
            }
        }
    } else if (entityName.find("敌人:") != std::string::npos) {
        // 处理敌人交互
        std::string enemyName = entityName.substr(3); // 移除"敌人: "前缀
        auto enemies = currentArea->getEnemies();
        for (auto enemy : enemies) {
            if (enemy->name == enemyName) {
                currentMessage_ = "攻击 " + enemyName + "! 敌人生命值: " + std::to_string(enemy->health);
                // 这里可以添加战斗逻辑
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
                    currentMessage_ = "拾取了 " + collectedItem.name + "!";
                    mapManager_->removeCollectibleFromCurrentArea(collectible);
                } else {
                    currentMessage_ = itemName + " 已经被拾取了。";
                }
                break;
            }
        }
    }
    
    UpdateDisplay();
}

void MapScreen::HandleNavigation() {
    // 导航逻辑可以在这里实现
}

void MapScreen::UpdateDisplay() {
    // 更新显示内容
    if (mapOverview_) mapOverview_->TakeFocus();
}
