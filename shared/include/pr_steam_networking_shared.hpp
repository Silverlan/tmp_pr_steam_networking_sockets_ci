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

// https://stackoverflow.com/a/7477384/2482983
// +5 for ports
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

#pragma pack(push,1)
struct HeaderData
{
	uint32_t messageId;
	uint32_t bodySize;
};
#pragma pack(pop)

class NetPacketReceiver
{
public:
	std::optional<NetPacket> ReceiveDataFragment(ISteamNetworkingMessage &msg);
private:
	HeaderData m_packetHeader = {};
	NetPacket m_packetBody;
	bool m_bReceivedHeader = false;
};

std::optional<NetPacket> NetPacketReceiver::ReceiveDataFragment(ISteamNetworkingMessage &msg)
{
	if(m_bReceivedHeader == false)
	{
		auto size = msg.GetSize();
		if(size != sizeof(m_packetHeader))
			return {};
		auto *data = msg.GetData();
		memcpy(&m_packetHeader,data,size);
		m_bReceivedHeader = true;
		return {};
	}
	// Receive body
	auto size = msg.GetSize();
	auto *data = msg.GetData();
	m_bReceivedHeader = false;
	m_packetBody = NetPacket{m_packetHeader.messageId,size};
	memcpy(m_packetBody->GetData(),data,size);
	return m_packetBody;
}

class ISteamNetworkingSockets;
class NetPacketDispatcher
{
public:
	bool SendData(ISteamNetworkingSockets &sns,uint32_t hConnection,pragma::networking::Protocol protocol,const void *data,uint32_t dataSize,pragma::networking::Error &outErr);
	bool SendPacket(ISteamNetworkingSockets &sns,uint32_t hConnection,pragma::networking::Protocol protocol,NetPacket &packet,pragma::networking::Error &outErr);
};
bool NetPacketDispatcher::SendPacket(
	ISteamNetworkingSockets &sns,uint32_t hConnection,
	pragma::networking::Protocol protocol,NetPacket &packet,pragma::networking::Error &outErr
)
{
	auto bodySize = packet->GetSize();
	HeaderData headerData {packet.GetMessageID(),bodySize};
	return SendData(sns,hConnection,protocol,&headerData,sizeof(headerData),outErr) &&
		SendData(sns,hConnection,protocol,packet->GetData(),bodySize,outErr);
}
bool NetPacketDispatcher::SendData(
	ISteamNetworkingSockets &sns,uint32_t hConnection,pragma::networking::Protocol protocol,const void *data,uint32_t dataSize,pragma::networking::Error &outErr
)
{
	auto sendFlags = get_send_flags(protocol);
	auto eResult = sns.SendMessageToConnection(hConnection,data,dataSize,sendFlags);
	switch(eResult)
	{
	case k_EResultOK:
		return true;
	case k_EResultInvalidParam:
		if(!hConnection)
			outErr = {pragma::networking::ErrorCode::InvalidConnectionHandle,"Invalid connection handle!"};
		else
			outErr = {pragma::networking::ErrorCode::MessageTooLarge,"Message is too big!"};
		break;
	case k_EResultInvalidState:
		outErr = {pragma::networking::ErrorCode::InvalidConnectionHandle,"Connection is in an invalid state!"};
		break;
	case k_EResultNoConnection:
		outErr = {pragma::networking::ErrorCode::InvalidConnectionHandle,"Connection has ended!"};
		break;
	case k_EResultIgnored:
		outErr = {pragma::networking::ErrorCode::GenericError,"Not ready to send message!"};
		break;
	case k_EResultLimitExceeded:
		outErr = {pragma::networking::ErrorCode::UnableToSendPacket,"Too much data on send queue!"};
		break;
	default:
		outErr = {pragma::networking::ErrorCode::UnableToSendPacket,"Unknown error!"};
		break;
	}
	return false;
}

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

static bool initialize_steam_game_networking_sockets(std::string &err)
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
static void kill_steam_game_networking_sockets()
{
	auto numInstances = get_net_lib_instance_count();
	assert(numInstances != 0);
	if(numInstances <= 0)
		return;
	if(numInstances == 1) // This is the last library instance
		GameNetworkingSockets_Kill();
	engine->SetConVar(cvarNameInstances,std::to_string(numInstances -1));
}

#endif
