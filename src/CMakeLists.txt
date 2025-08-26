cmake_minimum_required(VERSION 4.0)
project(CPP_MUD_OUC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(FTXUI)
add_executable(CPP_MUD_OUC core/main.cpp
        player/player.cpp
        player/player.h
        core/game.cpp
        core/game.h
        display/display.cpp
        display/display.h
        storage/storage.cpp
        storage/storage.h
        interaction/interaction.cpp
        interaction/interaction.h)


target_link_libraries(CPP_MUD_OUC
        PRIVATE ftxui::screen
        PRIVATE ftxui::dom
        PRIVATE ftxui::component
)