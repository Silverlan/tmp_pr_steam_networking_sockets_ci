cmake_minimum_required(VERSION 3.12)

set(PROJ_NAME_PREFIX pr_game_networking)
set(CONFIG_USE_STEAMWORKS_NETWORKING OFF)
add_subdirectory(client)
add_subdirectory(server)

set(PROJ_NAME_PREFIX pr_steam_networking)
set(CONFIG_USE_STEAMWORKS_NETWORKING ON)
add_subdirectory(steam_client)
add_subdirectory(steam_server)

set(CMAKE_CXX_STANDARD 17)
