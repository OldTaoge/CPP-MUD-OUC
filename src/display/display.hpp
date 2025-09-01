#ifndef DISPLAY_H
#define DISPLAY_H
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <map>
#include <string>
#include <vector>

class BaseScreen {
public:
    virtual ~BaseScreen() = default;

private:
    friend class ScreenManager;
    virtual ftxui::Component GetComponent() = 0;
};


class ScreenManager {
public:
    ScreenManager();
    ~ScreenManager();
    void mainloop();

private:
    void HandleMainMenuSelection(int selected_option);

    ftxui::ScreenInteractive screen_;
    std::map<std::string, BaseScreen*> screens_;
    std::string currentScreen_;
};

#endif //DISPLAY_H