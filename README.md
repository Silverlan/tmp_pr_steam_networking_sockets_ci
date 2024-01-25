[![Build Windows](https://github.com/Silverlan/pr_steam_networking_sockets/actions/workflows/build-windows-ci.yml/badge.svg)](https://github.com/Silverlan/pr_steam_networking_sockets/actions/workflows/build-windows-ci.yml) [![Build Linux](https://github.com/Silverlan/pr_steam_networking_sockets/actions/workflows/build-linux-ci.yml/badge.svg)](https://github.com/Silverlan/pr_steam_networking_sockets/actions/workflows/build-linux-ci.yml)

# Steam Networking Sockets
This is a binary module for the [Pragma Game Engine](https://github.com/Silverlan/pragma). For more information on binary modules, check out [this wiki article](https://wiki.pragma-engine.com/books/pragma-engine/page/binary-modules).

## Installation
To install this module, download one of the prebuilt binaries on the right and extract the archive over your Pragma installation.

There are two modules you can choose between: "game_networking" and "steam_networking".
The "steam_networking" module is disabled by default and can be enabled using the "CONFIG_ENABLE_STEAM_NETWORKING_SOCKETS" CMake option. This module requires steamworks!

To use the modules, you can use the "net_library" console command before starting a Pragma server (e.g. "net_library game_networking").
If you're using the "steam_networking" module, you should also make sure to enable the "sv_require_authentication" console command before starting a server, which will restrict the server to verified Steam users only.

## Developing

Please see the [wiki](https://wiki.pragma-engine.com/books/pragma-engine/page/binary-modules#bkmrk-building-modules) for instructions on how to develop and build the module.
