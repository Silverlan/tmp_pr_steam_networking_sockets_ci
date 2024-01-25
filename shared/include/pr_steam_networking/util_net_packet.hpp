#ifndef __PR_UTIL_NET_PACKET_HPP__
#define __PR_UTIL_NET_PACKET_HPP__

#include "pr_steam_networking/header_data.hpp"
#include "pr_steam_networking/common.hpp"
#include <sharedutils/netpacket.hpp>
#include <optional>

class BaseSteamNetworkingSocket;
class NetPacketReceiver {
  public:
	std::optional<NetPacket> ReceiveDataFragment(BaseSteamNetworkingSocket &sns, ISteamNetworkingMessage &msg);
  private:
	HeaderData m_packetHeader = {};
	NetPacket m_packetBody;
	bool m_bReceivedHeader = false;
};

class ISteamNetworkingSockets;
class NetPacketDispatcher {
  public:
	bool SendData(ISteamNetworkingSockets &sns, uint32_t hConnection, pragma::networking::Protocol protocol, const void *data, uint32_t dataSize, pragma::networking::Error &outErr);
	bool SendPacket(ISteamNetworkingSockets &sns, uint32_t hConnection, pragma::networking::Protocol protocol, NetPacket &packet, pragma::networking::Error &outErr);
};

#endif
