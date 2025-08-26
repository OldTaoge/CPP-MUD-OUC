cmake_minimum_required(VERSION 4.0)
project(CPP_MUD_OUC)

set(CMAKE_CXX_STANDARD 17)

add_executable(CPP_MUD_OUC src/core/main.cpp
        src/player/player.cpp
        src/player/player.h
        src/core/game.cpp
        src/core/game.h
        src/display/display.cpp
        src/display/display.h
        src/storage/storage.cpp
        src/storage/storage.h
        src/interaction/interaction.cpp
        src/interaction/interaction.h)
