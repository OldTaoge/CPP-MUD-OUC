cmake_minimum_required(VERSION 4.0)
project(CPP_MUD_OUC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(FTXUI-src)
add_executable(CPP_MUD_OUC core/main.cpp
        player/player.cpp
        player/player.h
        player/item.cpp
        player/item.h
        player/quest.cpp
        player/quest.h
        core/game.cpp
        core/game.h
        display/display.cpp
        display/display.hpp
        display/window_size_checker.cpp
        display/window_size_checker.hpp
        storage/storage.cpp
        storage/storage.h
        interaction/interaction.cpp
        interaction/interaction.h
        loader/loader.cpp
        loader/loader.h
        display/screens/mainmenu.cpp
        display/screens/mainmenu.hpp
        display/screens/gameplay.cpp
        display/screens/gameplay.hpp
        display/screens/illustrateMenu.cpp
        display/screens/illustrateMenu.hpp
        display/screens/settings.cpp
        display/screens/settings.hpp
        display/screens/inventory.cpp
        display/screens/inventory.hpp
        display/screens/questscreen.cpp
        display/screens/questscreen.hpp
        utils/utils.cpp
        utils/utils.hpp)

target_include_directories(CPP_MUD_OUC PRIVATE vendor)



target_link_libraries(CPP_MUD_OUC
        PRIVATE ftxui::screen
        PRIVATE ftxui::dom
        PRIVATE ftxui::component
)