// =============================================
// 文件: global_settings.cpp
// 描述: 全局设置管理实现
// =============================================

#include "global_settings.hpp"

// 静态成员定义
bool GlobalSettings::ai_enabled_ = false; // 默认禁用

bool GlobalSettings::IsAIEnabled() {
    return ai_enabled_;
}

void GlobalSettings::SetAIEnabled(bool enabled) {
    ai_enabled_ = enabled;
}
