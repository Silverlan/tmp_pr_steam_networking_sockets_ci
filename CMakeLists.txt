cmake_minimum_required(VERSION 3.12)

option(CONFIG_ENABLE_STEAM_NETWORKING_SOCKETS "If enabled, steam networking socket modules will be enabled. Requires steamworks." OFF)

set(PROJ_NAME_PREFIX pr_game_networking)
set(CONFIG_USE_STEAMWORKS_NETWORKING OFF)
add_subdirectory(client)
add_subdirectory(server)

if(${CONFIG_ENABLE_STEAM_NETWORKING_SOCKETS})
    set(PROJ_NAME_PREFIX pr_steam_networking)
    set(CONFIG_USE_STEAMWORKS_NETWORKING ON)
    add_subdirectory(steam_client)
    add_subdirectory(steam_server)
endif()

set(CMAKE_CXX_STANDARD 17)

set_target_properties(pr_game_networking_client PROPERTIES FOLDER modules/networking)
set_target_properties(pr_game_networking_server PROPERTIES FOLDER modules/networking)
if(${CONFIG_ENABLE_STEAM_NETWORKING_SOCKETS})
    set_target_properties(pr_steam_networking_client PROPERTIES FOLDER modules/networking)
    set_target_properties(pr_steam_networking_server PROPERTIES FOLDER modules/networking)
endif()

set_property(GLOBAL PROPERTY PRAGMA_MODULE_SKIP_TARGET_PROPERTY_FOLDER 1)
