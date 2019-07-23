#ifndef __PR_STEAM_NETWORKING_SHARED_HPP__
#define __PR_STEAM_NETWORKING_SHARED_HPP__

#undef ENABLE_STEAM_SERVER_SUPPORT
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#ifdef ENABLE_STEAM_SERVER_SUPPORT
#include <steam/steam_api.h>
#endif
#include <pragma/networking/enums.hpp>
#include <pragma/networking/error.hpp>
#include <pragma/engine.h>
#include <pragma/networkstate/networkstate.h>
#include <sharedutils/netpacket.hpp>
#include <sharedutils/util_library.hpp>

// https://stackoverflow.com/a/7477384/2482983
// +6 for ports
constexpr size_t MAX_IP_CHAR_LENGTH = 45 +6;

static int get_send_flags(pragma::networking::Protocol protocol)
{
	int sendFlags = k_nSteamNetworkingSend_NoNagle; // TODO: Allow caller to control nagle?
	switch(protocol)
	{
	case pragma::networking::Protocol::FastUnreliable:
		sendFlags |= k_nSteamNetworkingSend_Unreliable;
		break;
	case pragma::networking::Protocol::SlowReliable:
	default:
		sendFlags |= k_nSteamNetworkingSend_Reliable;
		break;
	}
	return sendFlags;
}

bool initialize_steam_game_networking_sockets(std::string &err);
void kill_steam_game_networking_sockets();

class BaseSteamNetworkingSocket
{
public:
	void Initialize();
	ISteamNetworkingSockets &GetSteamInterface() const;
	std::chrono::high_resolution_clock::duration GetDurationSinceStart(SteamNetworkingMicroseconds t) const;
	std::chrono::high_resolution_clock::time_point GetStartTime() const;
	static void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType,const char *msg);
private:
	ISteamNetworkingSockets *m_pInterface = nullptr;
	SteamNetworkingMicroseconds m_steamStartTime = 0;
	std::chrono::high_resolution_clock::time_point m_chronoStartTime;
};

#endif
