#include "pr_steam_networking/common.hpp"
#include <iostream>

void BaseSteamNetworkingSocket::Initialize()
{
	m_pInterface = SteamNetworkingSockets();
	m_steamStartTime = SteamNetworkingUtils()->GetLocalTimestamp();
	m_chronoStartTime = util::Clock::now();
	SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
}

ISteamNetworkingSockets &BaseSteamNetworkingSocket::GetSteamInterface() const { return *m_pInterface; }

util::Clock::duration BaseSteamNetworkingSocket::GetDurationSinceStart(SteamNetworkingMicroseconds t) const
{
	auto tDelta = t - m_steamStartTime;
	return std::chrono::microseconds {tDelta};
}
util::Clock::time_point BaseSteamNetworkingSocket::GetStartTime() const { return m_chronoStartTime; }
SteamNetworkingMicroseconds BaseSteamNetworkingSocket::GetSteamStartTime() const { return m_steamStartTime; }

void BaseSteamNetworkingSocket::DebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char *msg) { std::cout << "[Steam networking]: " << msg << std::endl; }
