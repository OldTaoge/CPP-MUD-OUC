// 引入屏幕显示相关的头文件
#include "../display/display.hpp"

// 主函数入口，argc 为参数数量，argv 为参数数组
int main(int argc, const char* argv[]) {
    // 创建屏幕管理器对象，用于管理主界面显示和事件循环
    auto sm = ScreenManager();

    // 设置控制台字符编码为 UTF-8，确保中文等字符正常显示
    system("chcp 65001");

    // 启动主事件循环，处理用户输入和界面刷新
    sm.mainloop();

    return 0;
}