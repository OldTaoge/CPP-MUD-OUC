//
// Created by Wentao on 2025/8/31.
//

#ifndef CPP_MUD_OUC_ILLUSTRATEMENU_HPP
#define CPP_MUD_OUC_ILLUSTRATEMENU_HPP
#include "../display.hpp"

class IllustrateMenu : public BaseScreen {
public:
    explicit IllustrateMenu(std::function<void(int)> on_select);

    ftxui::Component GetComponent() override;
};


#endif //CPP_MUD_OUC_ILLUSTRATEMENU_HPP