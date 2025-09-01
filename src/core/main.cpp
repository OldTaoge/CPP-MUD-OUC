#include "../display/display.hpp"
int main(int argc, const char* argv[]) {
    auto sm = ScreenManager();
    system("chcp 65001");
    sm.mainloop();
    return 0;
}