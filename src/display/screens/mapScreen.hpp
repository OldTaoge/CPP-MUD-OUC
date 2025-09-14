#ifndef MAPSCREEN_HPP
#define MAPSCREEN_HPP

#include "../display.hpp"
#include "../../core/map.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/event.hpp>

class MapScreen : public BaseScreen {
public:
    MapScreen(MapManager* mapManager);
    ~MapScreen() = default;
    
    void SetMapManager(MapManager* mapManager) { mapManager_ = mapManager; }
    
protected:
    ftxui::Component GetComponent() override;
    
private:
    ftxui::Component CreateMapOverview();
    ftxui::Component CreateAreaDetails();
    ftxui::Component CreateInteractionPanel();
    ftxui::Component CreateNavigationPanel();
    
    void HandleAreaSelection(int areaIndex);
    void HandleInteraction(int entityIndex);
    void HandleNavigation();
    
    void UpdateDisplay();
    
    MapManager* mapManager_;
    int selectedArea_;
    int selectedEntity_;
    std::string currentMessage_;
    
    ftxui::Component mapOverview_;
    ftxui::Component areaDetails_;
    ftxui::Component interactionPanel_;
    ftxui::Component navigationPanel_;
    
    std::vector<std::string> currentInteractables_;
};

#endif // MAPSCREEN_HPP
