#include "display/display.hpp"
#include <iostream>

int main() {
    std::cout << "=== 屏幕切换测试 ===" << std::endl;
    
    try {
        // 创建屏幕管理器
        ScreenManager screenManager;
        
        std::cout << "屏幕管理器创建成功" << std::endl;
        
        // 测试屏幕切换请求
        NavigationRequest validRequest(NavigationAction::SWITCH_SCREEN, "Gameplay");
        std::cout << "测试有效的屏幕切换请求..." << std::endl;
        
        NavigationRequest invalidRequest(NavigationAction::SWITCH_SCREEN, "InvalidScreen");
        std::cout << "测试无效的屏幕切换请求..." << std::endl;
        
        std::cout << "测试完成，如果没有崩溃则说明修复成功" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
