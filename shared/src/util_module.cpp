#include "pr_steam_networking/util_module.hpp"
#include "pr_steam_networking/common.hpp"
#include <pragma/engine.h>

extern DLLENGINE Engine *engine;
static constexpr const char *cvarNameInstances = "steam_networking_sockets_instances";
static int32_t get_net_lib_instance_count()
{
	// Internal console command to keep track of the number of steam networking library instances.
	// This is required because 'GameNetworkingSockets_Init' must only be called once per application,
	// however if a server is created and connected to locally, two library instances exist (server and client),
	// in which case we must ensure it's not called an additional time.
	auto cvInstanceCount = engine->RegisterConVar(cvarNameInstances,"0",ConVarFlags::Hidden,"");
	if(cvInstanceCount == nullptr)
		return -1;
	return engine->GetConVarInt(cvarNameInstances);
}

bool initialize_steam_game_networking_sockets(std::string &err)
{
	auto numInstances = get_net_lib_instance_count();
	if(numInstances < 0)
	{
		err = "Unable to initialize game networking sockets: An error has occurred previously!";
		return false;
	}
	if(numInstances == 0)
	{
		// This is the first library instance; Initialize the game networking sockets (This must only be done once per application)
		SteamDatagramErrMsg errMsg {};
		if(numInstances == -1 || GameNetworkingSockets_Init(nullptr,errMsg) == false)
		{
			err = "Unable to initialize game networking sockets: " +std::string{errMsg};
			engine->SetConVar(cvarNameInstances,std::to_string(-1));
			return false;
		}
	}
	engine->SetConVar(cvarNameInstances,std::to_string(numInstances +1));
	return true;
}
void kill_steam_game_networking_sockets()
{
	auto numInstances = get_net_lib_instance_count();
	assert(numInstances != 0);
	if(numInstances <= 0)
		return;
	if(numInstances == 1) // This is the last library instance
		GameNetworkingSockets_Kill();
	engine->SetConVar(cvarNameInstances,std::to_string(numInstances -1));
}
