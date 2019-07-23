#include "pr_module.hpp"
#include <pr_steam_networking/common.hpp>
#include <pr_steam_networking/util_net_packet.hpp>
#include <sharedutils/util_weak_handle.hpp>
#include <pragma/networking/iclient.hpp>
#include <pragma/networking/error.hpp>
#include <mathutil/umath.h>
#include <iostream>
#include <array>
#include <string>

#pragma optimize("",off)
class SteamClient
	: public pragma::networking::IClient,
	public NetPacketReceiver,public NetPacketDispatcher,
	private ISteamNetworkingSocketsCallbacks,
	public BaseSteamNetworkingSocket
{
public:
	bool Initialize(pragma::networking::Error &outErr);
	virtual std::string GetIdentifier() const override;
	virtual bool Connect(const std::string &ip,pragma::networking::Port port,pragma::networking::Error &outErr) override;
	virtual bool Disconnect(pragma::networking::Error &outErr) override;
	virtual bool SendPacket(pragma::networking::Protocol protocol,NetPacket &packet,pragma::networking::Error &outErr) override;
	virtual bool IsRunning() const override;
	virtual bool IsDisconnected() const override;
	virtual bool PollEvents(pragma::networking::Error &outErr) override;
	virtual uint16_t GetLatency() const override;
	virtual void SetTimeoutDuration(float duration) override;
	virtual std::optional<std::string> GetIP() const override;
	virtual std::optional<pragma::networking::Port> GetLocalTCPPort() const override;
	virtual std::optional<pragma::networking::Port> GetLocalUDPPort() const override;
	virtual void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *cbInfo) override;
protected:
	bool PollMessages(pragma::networking::Error &outErr);
private:
	HSteamNetConnection m_hConnection = k_HSteamNetConnection_Invalid;
	SteamNetworkingQuickConnectionStatus m_connectionStatus = {};
	bool m_bConnected = false;
};

bool SteamClient::Initialize(pragma::networking::Error &outErr)
{
	BaseSteamNetworkingSocket::Initialize();
	return true;
}

std::string SteamClient::GetIdentifier() const
{
	std::array<char,4'096> conName;
	if(GetSteamInterface().GetConnectionName(m_hConnection,conName.data(),conName.size()) == false)
		return "";
	return conName.data();
}

bool SteamClient::Connect(const std::string &ip,pragma::networking::Port port,pragma::networking::Error &outErr)
{
	SteamNetworkingIPAddr ipAddr {};
	if(ipAddr.ParseString((ip +':' +std::to_string(port)).c_str()) == false)
		return false;
	m_hConnection = GetSteamInterface().ConnectByIPAddress(ipAddr);
	return m_hConnection != k_HSteamNetConnection_Invalid;
}
bool SteamClient::Disconnect(pragma::networking::Error &outErr)
{
	// TODO: Reason, etc.
	auto result = GetSteamInterface().CloseConnection(m_hConnection,k_ESteamNetConnectionEnd_App_Min,"",false);
	GetSteamInterface().CloseConnection(m_hConnection,0,nullptr,false);
	m_hConnection = k_HSteamNetConnection_Invalid;
	m_bConnected = false;
	if(result == false)
		return result;
	OnDisconnected();
	return result;
}
bool SteamClient::SendPacket(pragma::networking::Protocol protocol,NetPacket &packet,pragma::networking::Error &outErr)
{
	return NetPacketDispatcher::SendPacket(GetSteamInterface(),m_hConnection,protocol,packet,outErr);
}
bool SteamClient::IsRunning() const
{
	// TODO
	return true;
}
bool SteamClient::IsDisconnected() const
{
	// TODO
	return false;
}
bool SteamClient::PollMessages(pragma::networking::Error &outErr)
{
	if(!m_hConnection)
		return true; // Nothing to poll
	std::array<ISteamNetworkingMessage*,256> incommingMessages;
	auto numMsgs = GetSteamInterface().ReceiveMessagesOnConnection(m_hConnection,incommingMessages.data(),incommingMessages.size());
	if(numMsgs < 0)
	{
		outErr = {pragma::networking::ErrorCode::GenericError,"Invalid connection!"};
		return false;
	}
	for(auto i=decltype(numMsgs){0u};i<numMsgs;++i)
	{
		auto *pIncomingMsg = incommingMessages.at(i);
		auto packet = ReceiveDataFragment(*this,*pIncomingMsg);
		if(packet.has_value())
			HandlePacket(*packet);
		pIncomingMsg->Release();
	}
	return true;
}
bool SteamClient::PollEvents(pragma::networking::Error &outErr)
{
	auto success = PollMessages(outErr);
	GetSteamInterface().RunCallbacks(this);
	GetSteamInterface().GetQuickConnectionStatus(m_hConnection,&m_connectionStatus);
	return success;
}
uint16_t SteamClient::GetLatency() const
{
	return m_connectionStatus.m_nPing;
}
void SteamClient::SetTimeoutDuration(float duration) {}
std::optional<std::string> SteamClient::GetIP() const
{
	SteamNetConnectionInfo_t info;
	if(GetSteamInterface().GetConnectionInfo(m_hConnection,&info) == false)
		return {};
	auto &address = info.m_addrRemote;
	std::array<char,MAX_IP_CHAR_LENGTH> ip;
	address.ToString(ip.data(),ip.size(),true);
	return ip.data();
}
std::optional<pragma::networking::Port> SteamClient::GetLocalTCPPort() const {return {};}
std::optional<pragma::networking::Port> SteamClient::GetLocalUDPPort() const {return {};}

void SteamClient::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *cbInfo)
{
	if(!m_hConnection)
		return;
	switch(cbInfo->m_info.m_eState)
	{
	case k_ESteamNetworkingConnectionState_None:
	case k_ESteamNetworkingConnectionState_Connecting:
	case k_ESteamNetworkingConnectionState_FindingRoute:
		break;
	case k_ESteamNetworkingConnectionState_Connected:
		if(m_bConnected == false)
		{
			m_bConnected = true;
			OnConnected();
		}
		break;
	case k_ESteamNetworkingConnectionState_ClosedByPeer:
	case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
		GetSteamInterface().CloseConnection(cbInfo->m_hConn,0,nullptr,false);
		m_hConnection = k_HSteamNetConnection_Invalid;
		m_bConnected = false;

		OnConnectionClosed();
		break;
	}
}

class NetworkState;
extern "C"
{
	PRAGMA_EXPORT bool pragma_attach(std::string &err)
	{
		return initialize_steam_game_networking_sockets(err);
	}
	PRAGMA_EXPORT void pragma_detach()
	{
		kill_steam_game_networking_sockets();
	}
	PRAGMA_EXPORT void initialize_game_client(NetworkState &nw,std::unique_ptr<pragma::networking::IClient> &outClient)
	{
		auto cl = std::make_unique<SteamClient>();
		pragma::networking::Error err;
		if(cl->Initialize(err))
			outClient = std::move(cl);
		else
			outClient = nullptr;
	}
};
#pragma optimize("",on)
