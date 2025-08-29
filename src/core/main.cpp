#include "../display/display.hpp"
int main(int argc, const char* argv[]) {
    auto sm = ScreenManager();
    sm.mainloop();
    return 0;
}