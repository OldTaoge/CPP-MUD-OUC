//
// Created for Save Selection Screen
//

#include "save_select.hpp"
#include "../window_size_checker.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <iostream>

using namespace ftxui;

SaveSelectScreen::SaveSelectScreen(Game* game, SaveSelectMode mode)
    : game_(game), mode_(mode), selectedSlot_(0), showingInput_(false), 
      showingDeleteConfirm_(false), deleteSlotIndex_(-1), showingStatusMessage_(false) {
    RefreshSaveList();
    
    // 创建主容器
    container_ = Container::Vertical({});
    
    // 创建主渲染器
    component_ = Renderer(container_, [this] {
        return CreateMainRenderer();
    });
    
    // 添加键盘处理
    component_ = CatchEvent(component_, [this](Event event) {
        return HandleKeyboardInput(event);
    });
    
    // 使用窗口大小检测包装组件
    component_ = WindowSizeChecker::Make(component_, 120, 50);
}

Component SaveSelectScreen::GetComponent() {
    return component_;
}

void SaveSelectScreen::SetMode(SaveSelectMode mode) {
    mode_ = mode;
    RefreshSaveList();
}

void SaveSelectScreen::RefreshSaveList() {
    saveSlots_.clear();
    
    // 获取所有存档文件
    auto saveFiles = game_->getSaveFiles();
    
    // 添加现有存档
    for (const auto& fileName : saveFiles) {
        if (game_->saveExists(fileName)) {
            auto saveInfo = game_->getSaveInfo(fileName);
            saveSlots_.emplace_back(fileName, saveInfo);
        }
    }
    
    // 在保存模式下，添加一个"新建存档"选项
    if (mode_ == SaveSelectMode::SAVE) {
        SaveSlotInfo newSlot;
        newSlot.fileName = "[新建存档]";
        newSlot.isEmpty = true;
        saveSlots_.insert(saveSlots_.begin(), newSlot);
    }
    
    // 确保选中索引有效
    if (selectedSlot_ >= static_cast<int>(saveSlots_.size())) {
        selectedSlot_ = std::max(0, static_cast<int>(saveSlots_.size()) - 1);
    }
}

Element SaveSelectScreen::CreateMainRenderer() {
    if (showingInput_) {
        return CreateInputDialog();
    }
    
    if (showingDeleteConfirm_) {
        return CreateDeleteConfirmDialog();
    }
    
    Elements elements;
    
    // 标题
    elements.push_back(RenderModeTitle());
    elements.push_back(separator());
    
    // 状态消息
    if (showingStatusMessage_ && !statusMessage_.empty()) {
        elements.push_back(text(statusMessage_) | hcenter | color(Color::Green) | bold);
        elements.push_back(separator());
    }
    
    // 存档列表
    if (saveSlots_.empty() && mode_ == SaveSelectMode::LOAD) {
        elements.push_back(text("没有找到存档文件") | hcenter | color(Color::Red));
    } else {
        Elements slotElements;
        for (size_t i = 0; i < saveSlots_.size(); ++i) {
            bool isSelected = (i == static_cast<size_t>(selectedSlot_));
            slotElements.push_back(RenderSaveSlot(saveSlots_[i], static_cast<int>(i), isSelected));
        }
        elements.push_back(vbox(slotElements));
    }
    
    elements.push_back(separator());
    elements.push_back(RenderInstructions());
    
    return vbox(elements) | border | hcenter | vcenter;
}

Element SaveSelectScreen::RenderModeTitle() {
    std::string title = (mode_ == SaveSelectMode::LOAD) ? "加载存档" : "保存存档";
    return text(title) | bold | hcenter | color(Color::Cyan);
}

Element SaveSelectScreen::RenderSaveSlot(const SaveSlotInfo& slot, int index, bool isSelected) {
    Elements slotContent;
    
    if (slot.isEmpty) {
        // 新建存档选项
        slotContent.push_back(text("+ 新建存档") | color(Color::Green));
    } else {
        // 存档信息
        std::stringstream ss;
        ss << "[" << std::setw(2) << std::setfill('0') << (index + 1) << "] ";
        
        // 显示存档文件名（去掉.json扩展名）
        std::string displayFileName = slot.fileName;
        if (displayFileName.length() > 5 && displayFileName.substr(displayFileName.length() - 5) == ".json") {
            displayFileName = displayFileName.substr(0, displayFileName.length() - 5);
        }
        ss << displayFileName;
        
        slotContent.push_back(text(ss.str()) | bold | color(Color::Cyan));
        
        // 玩家信息
        if (!slot.saveInfo.playerName.empty()) {
            std::stringstream playerSs;
            playerSs << "玩家: " << slot.saveInfo.playerName;
            if (slot.saveInfo.level > 0) {
                playerSs << " (等级 " << slot.saveInfo.level << ")";
            }
            slotContent.push_back(text(playerSs.str()) | color(Color::White));
        }
        
        // 详细信息
        Elements details;
        if (!slot.saveInfo.saveTime.empty()) {
            details.push_back(text("保存时间: " + slot.saveInfo.saveTime) | color(Color::GrayLight));
        }
        
        std::stringstream detailSs;
        detailSs << "位置: (" << slot.saveInfo.x << ", " << slot.saveInfo.y << ")";
        if (slot.saveInfo.teamSize > 0) {
            detailSs << " | 队伍: " << slot.saveInfo.teamSize << "人";
        }
        if (slot.saveInfo.inventorySize > 0) {
            detailSs << " | 背包: " << slot.saveInfo.inventorySize << "件";
        }
        details.push_back(text(detailSs.str()) | color(Color::GrayLight));
        
        slotContent.push_back(vbox(details));
    }
    
    auto slotElement = vbox(slotContent);
    
    // 选中状态样式
    if (isSelected) {
        slotElement = slotElement | bgcolor(Color::Blue) | color(Color::White);
    }
    
    return slotElement | border;
}

Element SaveSelectScreen::RenderInstructions() {
    Elements instructions;
    
    if (mode_ == SaveSelectMode::LOAD) {
        instructions.push_back(text("↑↓: 选择存档  Enter: 加载  Del: 删除  Esc: 返回") | hcenter);
    } else {
        instructions.push_back(text("↑↓: 选择存档  Enter: 保存  Del: 删除  Esc: 返回") | hcenter);
    }
    
    return vbox(instructions) | color(Color::GrayLight);
}

Element SaveSelectScreen::CreateInputDialog() {
    Elements dialog;
    
    dialog.push_back(text("请输入存档名称:") | hcenter | bold);
    dialog.push_back(separator());
    
    // 输入框 - 预留更多空间
    std::string displayText = inputSaveName_;
    if (displayText.empty()) {
        displayText = "在此输入存档名称...";
    }
    
    // 创建输入框，预留足够的空间（支持中文显示）
    auto inputBox = text("名称: " + displayText) | 
                   border | 
                   size(WIDTH, GREATER_THAN, 60) |  // 增加宽度以支持中文
                   size(HEIGHT, EQUAL, 3) |         // 高度3行
                   hcenter;
    
    dialog.push_back(inputBox);
    
    dialog.push_back(separator());
    
    // 添加按钮区域
    Elements buttons;
    buttons.push_back(text("[确认]") | color(Color::Green) | bold);
    buttons.push_back(text("  ") | color(Color::White));  // 空格分隔
    buttons.push_back(text("[取消]") | color(Color::Red) | bold);
    
    dialog.push_back(hbox(buttons) | hcenter);
    dialog.push_back(separator());
    dialog.push_back(text("支持中文输入  Enter: 确认  Esc: 取消") | hcenter | color(Color::GrayLight));
    
    return vbox(dialog) | border | hcenter | vcenter | bgcolor(Color::Black);
}

Element SaveSelectScreen::CreateDeleteConfirmDialog() {
    Elements dialog;
    
    dialog.push_back(text("确认删除存档?") | hcenter | bold | color(Color::Red));
    dialog.push_back(separator());
    
    if (deleteSlotIndex_ >= 0 && deleteSlotIndex_ < static_cast<int>(saveSlots_.size())) {
        const auto& slot = saveSlots_[deleteSlotIndex_];
        if (!slot.isEmpty) {
            dialog.push_back(text("存档: " + slot.saveInfo.playerName) | hcenter);
            dialog.push_back(text("时间: " + slot.saveInfo.saveTime) | hcenter | color(Color::GrayLight));
        }
    }
    
    dialog.push_back(separator());
    
    // 添加按钮区域
    Elements buttons;
    buttons.push_back(text("[确认删除]") | color(Color::Red) | bold);
    buttons.push_back(text("  ") | color(Color::White));  // 空格分隔
    buttons.push_back(text("[取消]") | color(Color::Green) | bold);
    
    dialog.push_back(hbox(buttons) | hcenter);
    dialog.push_back(separator());
    dialog.push_back(text("Y: 确认删除  N/Esc: 取消") | hcenter | color(Color::GrayLight));
    
    return vbox(dialog) | border | hcenter | vcenter | bgcolor(Color::Black);
}

bool SaveSelectScreen::HandleKeyboardInput(Event event) {
    if (showingInput_) {
        return HandleInputDialog(event);
    }
    
    if (showingDeleteConfirm_) {
        return HandleDeleteConfirmDialog(event);
    }
    
    // 主界面键盘处理
    if (event == Event::ArrowUp) {
        if (selectedSlot_ > 0) {
            selectedSlot_--;
        }
        return true;
    }
    
    if (event == Event::ArrowDown) {
        if (selectedSlot_ < static_cast<int>(saveSlots_.size()) - 1) {
            selectedSlot_++;
        }
        return true;
    }
    
    if (event == Event::Return) {
        HandleSaveSlotAction(selectedSlot_);
        return true;
    }
    
    if (event == Event::Delete) {
        if (selectedSlot_ >= 0 && selectedSlot_ < static_cast<int>(saveSlots_.size()) &&
            !saveSlots_[selectedSlot_].isEmpty) {
            ShowDeleteConfirmation(selectedSlot_);
        }
        return true;
    }
    
    if (event == Event::Escape) {
        HandleBack();
        return true;
    }
    
    // 清除状态消息
    if (showingStatusMessage_) {
        showingStatusMessage_ = false;
        statusMessage_.clear();
        return true;
    }
    
    return false;
}

bool SaveSelectScreen::HandleInputDialog(Event event) {
    if (event == Event::Escape) {
        showingInput_ = false;
        inputSaveName_.clear();
        return true;
    }
    
    if (event == Event::Return) {
        if (!inputSaveName_.empty()) {
            std::string fileName = inputSaveName_ + ".json";
            SaveToSlot(fileName);
        }
        showingInput_ = false;
        inputSaveName_.clear();
        return true;
    }
    
    if (event == Event::Backspace) {
        if (!inputSaveName_.empty()) {
            inputSaveName_.pop_back();
        }
        return true;
    }
    
    // 处理字符输入 - 支持中文字符
    if (event.is_character()) {
        std::string charStr = event.character();
        
        // 调试信息：输出输入的字符（可选）
        // std::cout << "输入字符: " << charStr << " (长度: " << charStr.length() << ")" << std::endl;
        
        // 允许所有非控制字符输入（包括中文）
        // 只排除一些特殊控制字符
        bool isValidChar = true;
        
        if (charStr.length() == 1) {
            char ch = charStr[0];
            // 排除一些控制字符
            if (ch < 32 && ch != 9) { // 排除控制字符，但保留制表符
                isValidChar = false;
            }
        }
        
        if (isValidChar) {
            inputSaveName_ += charStr;
            // std::cout << "当前输入: " << inputSaveName_ << std::endl;
        }
        return true;
    }
    
    return false;
}

bool SaveSelectScreen::HandleDeleteConfirmDialog(Event event) {
    if (event == Event::Escape || event.character() == "n" || event.character() == "N") {
        showingDeleteConfirm_ = false;
        deleteSlotIndex_ = -1;
        return true;
    }
    
    if (event.character() == "y" || event.character() == "Y") {
        if (deleteSlotIndex_ >= 0 && deleteSlotIndex_ < static_cast<int>(saveSlots_.size())) {
            const auto& slot = saveSlots_[deleteSlotIndex_];
            if (!slot.isEmpty) {
                DeleteSaveFile(slot.fileName);
            }
        }
        showingDeleteConfirm_ = false;
        deleteSlotIndex_ = -1;
        return true;
    }
    
    return false;
}

void SaveSelectScreen::HandleSaveSlotAction(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= static_cast<int>(saveSlots_.size())) {
        return;
    }
    
    const auto& slot = saveSlots_[slotIndex];
    
    if (mode_ == SaveSelectMode::LOAD) {
        if (!slot.isEmpty) {
            LoadSelectedSave(slot.fileName);
        }
    } else { // SaveSelectMode::SAVE
        if (slot.isEmpty) {
            // 新建存档
            ShowSaveNameInput();
        } else {
            // 覆盖现有存档
            SaveToSlot(slot.fileName);
        }
    }
}

void SaveSelectScreen::HandleBack() {
    if (navigation_callback_) {
        navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "MainMenu"));
    }
}

void SaveSelectScreen::ShowSaveNameInput() {
    // 刷新存档列表以确保显示最新的存档
    RefreshSaveList();
    
    showingInput_ = true;
    inputSaveName_.clear();
}

void SaveSelectScreen::ShowDeleteConfirmation(int slotIndex) {
    // 刷新存档列表以确保显示最新的存档信息
    RefreshSaveList();
    
    showingDeleteConfirm_ = true;
    deleteSlotIndex_ = slotIndex;
}

void SaveSelectScreen::LoadSelectedSave(const std::string& fileName) {
    // 调用游戏的加载功能
    game_->LoadGame(fileName);
    
    // 切换到游戏界面
    if (navigation_callback_) {
        navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Gameplay"));
    }
}

void SaveSelectScreen::SaveToSlot(const std::string& fileName) {
    // 调试信息：输出文件名（可选）
    // std::cout << "保存存档，文件名: " << fileName << std::endl;
    
    // 尝试转换文件名编码（如果可能的话）
    std::string finalFileName = fileName;
    
    // 调用游戏的保存功能
    game_->SaveGame(finalFileName);
    
    // 刷新列表
    RefreshSaveList();
    
    // 显示保存成功消息
    statusMessage_ = "存档保存成功: " + finalFileName;
    showingStatusMessage_ = true;
    
    // 保存成功后不立即返回主菜单，让用户看到保存结果
    // 用户可以通过按Esc键返回主菜单
}

void SaveSelectScreen::DeleteSaveFile(const std::string& fileName) {
    if (game_->deleteSave(fileName)) {
        RefreshSaveList();
        // 调整选中索引
        if (selectedSlot_ >= static_cast<int>(saveSlots_.size()) && selectedSlot_ > 0) {
            selectedSlot_--;
        }
        
        // 显示删除成功消息
        statusMessage_ = "存档删除成功: " + fileName;
        showingStatusMessage_ = true;
    } else {
        // 显示删除失败消息
        statusMessage_ = "存档删除失败: " + fileName;
        showingStatusMessage_ = true;
    }
}
