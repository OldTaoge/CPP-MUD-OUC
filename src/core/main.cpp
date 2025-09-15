// =============================================
// 文件: main.cpp
// 描述: 程序入口，初始化字符集并启动屏幕管理主循环。
// 注意: Windows 控制台使用 chcp 65001 切换到 UTF-8，确保中文显示正常。
// =============================================
// 引入屏幕显示相关的头文件
#include "../display/display.hpp"

// 主函数入口，argc 为参数数量，argv 为参数数组
int main(int argc, const char* argv[]) {
    // 创建屏幕管理器对象，用于管理主界面显示和事件循环
    auto screenManager = ScreenManager();

    // 设置控制台字符编码为 UTF-8，确保中文等字符正常显示
    system("chcp 65001");

    // 启动主事件循环，处理用户输入和界面刷新
    screenManager.mainloop();

    return 0;
}