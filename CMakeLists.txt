cmake_minimum_required(VERSION 3.12)

include("../CMakeShared.txt")

set(PROJ_NAME pr_steam_networking)

project(${PROJ_NAME} CXX)

# set(PRECOMPILED_HEADER "stdafx")

link_generic_module_libraries()
register_pragma_module(${PROJ_NAME})

set(Protobuf_SRC_ROOT_FOLDER ${CMAKE_CURRENT_LIST_DIR}/third_party/protobuf/src CACHE STRING "" FORCE)
set(Protobuf_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/third_party/protobuf/src CACHE STRING "" FORCE)
set(Protobuf_LIBRARIES ${CMAKE_CURRENT_LIST_DIR}/third_party/protobuf/ CACHE STRING "" FORCE)

set(Protobuf_USE_STATIC_LIBS ON CACHE BOOL ON FORCE)

add_subdirectory(third_party/gamenetworkingsockets)
