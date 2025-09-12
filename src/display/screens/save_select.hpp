//
// Created for Save Selection Screen
//

#ifndef CPP_MUD_OUC_SAVE_SELECT_HPP
#define CPP_MUD_OUC_SAVE_SELECT_HPP

#include "../display.hpp"
#include "../../core/game.h"
#include <vector>
#include <string>

// 存档选择模式
enum class SaveSelectMode {
    LOAD,   // 加载存档模式
    SAVE    // 保存存档模式
};

// 存档信息结构
struct SaveSlotInfo {
    std::string fileName;
    GameSave::SaveInfo saveInfo;
    bool isEmpty;
    bool isSelected;
    
    SaveSlotInfo() : isEmpty(true), isSelected(false) {}
    SaveSlotInfo(const std::string& name, const GameSave::SaveInfo& info) 
        : fileName(name), saveInfo(info), isEmpty(false), isSelected(false) {}
};

class SaveSelectScreen : public BaseScreen {
public:
    SaveSelectScreen(Game* game, SaveSelectMode mode = SaveSelectMode::LOAD);
    
    // 实现基类的虚函数
    ftxui::Component GetComponent() override;
    
    // 设置模式
    void SetMode(SaveSelectMode mode);
    
    // 刷新存档列表
    void RefreshSaveList();

private:
    void HandleSaveSlotAction(int slotIndex);
    void HandleBack();
    
    // UI 渲染相关
    ftxui::Element CreateMainRenderer();
    ftxui::Element RenderSaveSlot(const SaveSlotInfo& slot, int index, bool isSelected);
    ftxui::Element RenderModeTitle();
    ftxui::Element RenderInstructions();
    ftxui::Element CreateInputDialog();
    ftxui::Element CreateDeleteConfirmDialog();
    
    // 存档操作
    void LoadSelectedSave(const std::string& fileName);
    void SaveToSlot(const std::string& fileName);
    void DeleteSaveFile(const std::string& fileName);
    
    // 输入处理
    void ShowSaveNameInput();
    void ShowDeleteConfirmation(int slotIndex);
    bool HandleKeyboardInput(ftxui::Event event);
    bool HandleInputDialog(ftxui::Event event);
    bool HandleDeleteConfirmDialog(ftxui::Event event);
    
    Game* game_;
    SaveSelectMode mode_;
    std::vector<SaveSlotInfo> saveSlots_;
    int selectedSlot_;
    bool showingInput_;
    bool showingDeleteConfirm_;
    int deleteSlotIndex_;
    std::string inputSaveName_;
    bool showingStatusMessage_;
    std::string statusMessage_;
    
    ftxui::Component component_;
    ftxui::Component container_;
    ftxui::Component inputComponent_;
    ftxui::Component deleteConfirmComponent_;
};

#endif //CPP_MUD_OUC_SAVE_SELECT_HPP
