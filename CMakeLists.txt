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

#set(DEPENDENCY_VALVE_GAMENETWORKINGSOCKETS_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party/gamenetworkingsockets/include CACHE PATH "" FORCE)
# set(DEPENDENCY_OPENSSL_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party/openssl/include CACHE PATH "" FORCE)

set(CMAKE_CXX_STANDARD 17)

set_target_properties(pr_game_networking_client PROPERTIES FOLDER modules/networking)
set_target_properties(pr_game_networking_server PROPERTIES FOLDER modules/networking)
if(${CONFIG_ENABLE_STEAM_NETWORKING_SOCKETS})
    set_target_properties(pr_steam_networking_client PROPERTIES FOLDER modules/networking)
    set_target_properties(pr_steam_networking_server PROPERTIES FOLDER modules/networking)
endif()

# add_subdirectory(third_party/gamenetworkingsockets)
# add_dependencies(pr_game_networking_client)
# add_dependencies(pr_game_networking_server)

set_property(GLOBAL PROPERTY PRAGMA_MODULE_SKIP_TARGET_PROPERTY_FOLDER 1)
#set_target_properties(GameNetworkingSockets PROPERTIES FOLDER modules/networking)
#set_target_properties(example_chat PROPERTIES FOLDER modules/networking)
#set_target_properties(test_crypto PROPERTIES FOLDER modules/networking)
