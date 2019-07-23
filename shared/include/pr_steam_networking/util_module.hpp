#ifndef __PR_MODULE_HPP__
#define __PR_MODULE_HPP__

#include <string>

bool initialize_steam_game_networking_sockets(std::string &err);
void kill_steam_game_networking_sockets();

#endif
