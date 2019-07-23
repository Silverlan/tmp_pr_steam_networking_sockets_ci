#include "pr_steam_networking/common.hpp"
#include <iostream>

void BaseSteamNetworkingSocket::Initialize()
{
	m_pInterface = SteamNetworkingSockets();
	m_steamStartTime = SteamNetworkingUtils()->GetLocalTimestamp();
	m_chronoStartTime = std::chrono::high_resolution_clock::now();
	SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg,DebugOutput);
}

ISteamNetworkingSockets &BaseSteamNetworkingSocket::GetSteamInterface() const {return *m_pInterface;}

std::chrono::high_resolution_clock::duration BaseSteamNetworkingSocket::GetDurationSinceStart(SteamNetworkingMicroseconds t) const
{
	auto tDelta = t -m_steamStartTime;
	return std::chrono::microseconds{tDelta};
}
std::chrono::high_resolution_clock::time_point BaseSteamNetworkingSocket::GetStartTime() const {return m_chronoStartTime;}

void BaseSteamNetworkingSocket::DebugOutput(ESteamNetworkingSocketsDebugOutputType eType,const char *msg)
{
	// TODO
	std::cout<<"Debug Output: "<<msg<<std::endl;
}
