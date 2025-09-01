//
// Created by Wentao on 2025/8/31.
//

#ifndef CPP_MUD_OUC_ILLUSTRATEMENU_HPP
#define CPP_MUD_OUC_ILLUSTRATEMENU_HPP
#include "../display.hpp"

class IllustrateMenu : public BaseScreen {
public:
    IllustrateMenu();

    ftxui::Component GetComponent() override;
    
private:
    void HandleSelection(int selected_option);
    
    ftxui::Component component_;
    ftxui::Element content_element_; // 缓存内容元素
};


#endif //CPP_MUD_OUC_ILLUSTRATEMENU_HPP