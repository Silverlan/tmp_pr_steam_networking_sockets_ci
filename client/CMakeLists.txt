cmake_minimum_required(VERSION 3.12)

include("../../CMakeShared.txt")

set(PROJ_NAME ${PROJ_NAME_PREFIX}_client)

project(${PROJ_NAME} CXX)

# set(PRECOMPILED_HEADER "stdafx")

add_include_dir(networkmanager)
add_include_dir(glm)

if(CONFIG_USE_STEAMWORKS_NETWORKING MATCHES ON)
	add_definitions(-DUSE_STEAMWORKS_NETWORKING)

	link_external_library(steamworks)
else()
	link_external_library(valve_gamenetworkingsockets)
	link_external_library(openssl)
	link_external_library(protobuf)
endif()

link_external_library(client)

add_source_path("${CMAKE_CURRENT_LIST_DIR}/../shared")

link_generic_module_libraries()
register_pragma_module(${PROJ_NAME})
