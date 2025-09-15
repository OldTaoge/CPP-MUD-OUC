// =============================================
// 文件: global_settings.hpp
// 描述: 全局设置管理，提供简单的设置状态访问
// =============================================

#ifndef CPP_MUD_OUC_GLOBAL_SETTINGS_HPP
#define CPP_MUD_OUC_GLOBAL_SETTINGS_HPP

class GlobalSettings {
public:
    // 获取AI建议开关状态
    static bool IsAIEnabled();
    
    // 设置AI建议开关状态
    static void SetAIEnabled(bool enabled);
    
private:
    static bool ai_enabled_;
};

#endif //CPP_MUD_OUC_GLOBAL_SETTINGS_HPP
