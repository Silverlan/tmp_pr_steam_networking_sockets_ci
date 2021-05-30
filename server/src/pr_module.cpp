#include "pr_module.hpp"
#include <pr_steam_networking/common.hpp>
#include <pr_steam_networking/util_net_packet.hpp>
#include <sharedutils/util_weak_handle.hpp>
#include <pragma/networking/iserver.hpp>
#include <pragma/networking/iserver_client.hpp>
#include <pragma/networking/error.hpp>
#include <pragma/networking/ip_address.hpp>
#include <pragma/console/convars.h>
#include <mathutil/umath.h>
#include <iostream>
#include <array>
#include <string>

namespace pragma::networking
{
	class SteamServer;
	class SteamServerClient
		: public pragma::networking::IServerClient,
		public NetPacketReceiver,public NetPacketDispatcher
	{
	public:
		SteamServerClient(SteamServer &sv,HSteamNetConnection con);
		virtual bool Drop(pragma::networking::DropReason reason,pragma::networking::Error &outErr) override;
		virtual uint16_t GetLatency() const override;
		virtual std::string GetIdentifier() const override;
		virtual std::optional<std::string> GetIP() const override;
		virtual std::optional<pragma::networking::Port> GetPort() const override;
		virtual bool IsListenServerHost() const override;
		virtual bool SendPacket(pragma::networking::Protocol protocol,NetPacket &packet,pragma::networking::Error &outErr) override;
	private:
		ISteamNetworkingSockets &GetInterface() const;
		HSteamNetConnection m_hConnection = k_HSteamNetConnection_Invalid;
		SteamServer &m_server;
	};
};

pragma::networking::SteamServerClient::SteamServerClient(SteamServer &sv,HSteamNetConnection con)
	: m_hConnection{con},m_server{sv}
{}

bool pragma::networking::SteamServerClient::Drop(pragma::networking::DropReason reason,pragma::networking::Error &outErr)
{
	if(m_hConnection == k_HSteamNetConnection_Invalid)
		return true;
	if(GetInterface().CloseConnection(m_hConnection,k_ESteamNetConnectionEnd_App_Min +umath::to_integral(reason),pragma::networking::drop_reason_to_string(reason).c_str(),true) == false)
	{
		outErr = {pragma::networking::ErrorCode::UnableToDropClient,"Unable to drop client!"};
		return false;
	}
	m_hConnection = k_HSteamNetConnection_Invalid;
	return true;
}
uint16_t pragma::networking::SteamServerClient::GetLatency() const
{
	return 0; // TODO
}
std::string pragma::networking::SteamServerClient::GetIdentifier() const
{
	std::array<char,4'096> conName;
	if(GetInterface().GetConnectionName(m_hConnection,conName.data(),conName.size()) == false)
		return "";
	return conName.data();
}
std::optional<std::string> pragma::networking::SteamServerClient::GetIP() const
{
	SteamNetConnectionInfo_t info;
	if(GetInterface().GetConnectionInfo(m_hConnection,&info) == false)
		return {};
	auto &address = info.m_addrRemote;
	std::array<char,MAX_IP_CHAR_LENGTH> ip;
	address.ToString(ip.data(),ip.size(),true);
	return ip.data();
}
std::optional<pragma::networking::Port> pragma::networking::SteamServerClient::GetPort() const {return {};}
bool pragma::networking::SteamServerClient::IsListenServerHost() const
{
	return false; // TODO
}
bool pragma::networking::SteamServerClient::SendPacket(pragma::networking::Protocol protocol,NetPacket &packet,pragma::networking::Error &outErr)
{
	return NetPacketDispatcher::SendPacket(GetInterface(),m_hConnection,protocol,packet,outErr);
}

/////////////

namespace pragma::networking
{
	class SteamServer
		: public pragma::networking::IServer,
#ifndef USE_STEAMWORKS_NETWORKING
		private ISteamNetworkingSocketsCallbacks,
#endif
		public BaseSteamNetworkingSocket
	{
	public:
		virtual bool DoShutdown(pragma::networking::Error &outErr) override;
		virtual bool Heartbeat() override;
		virtual std::optional<std::string> GetHostIP() const override;
		virtual std::optional<pragma::networking::Port> GetHostPort() const override;
		virtual std::optional<uint64_t> GetSteamId() const override;
		virtual bool IsPeerToPeer() const override;
		virtual bool PollEvents(pragma::networking::Error &outErr) override;
		virtual void SetTimeoutDuration(float duration) override;
#ifdef USE_STEAMWORKS_NETWORKING
		virtual std::string GetNetworkLayerIdentifier() const override {return "steam_networking";}
#else
		virtual std::string GetNetworkLayerIdentifier() const override {return "game_networking";}
#endif
		ISteamNetworkingSockets &GetInterface() const;

#ifndef USE_STEAMWORKS_NETWORKING
		virtual void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *info) override;
#endif
	protected:
		bool PollMessages(pragma::networking::Error &outErr);
		virtual bool DoStart(Error &outErr,uint16_t port,bool useP2PIfAvailable=false) override;
	private:
#ifdef USE_STEAMWORKS_NETWORKING
		STEAM_CALLBACK(SteamServer,OnConnectionStatusChanged,SteamNetConnectionStatusChangedCallback_t);
#else
		void OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pCallback);
#endif
		HSteamListenSocket m_hListenSock = 0;
		std::unordered_map<HSteamNetConnection,pragma::networking::SteamServerClient*> m_conHandleToClient = {};
		pragma::networking::Error m_statusError = {};
		// Only enabled if peer-to-peer
		std::optional<CSteamID> m_steamId = {};
	};
};

bool pragma::networking::SteamServer::DoStart(Error &outErr,uint16_t port,bool useP2PIfAvailable)
{
	BaseSteamNetworkingSocket::Initialize();
#ifndef USE_STEAMWORKS_NETWORKING
	useP2PIfAvailable = false;
#endif

	if(useP2PIfAvailable == false)
	{
		SteamNetworkingIPAddr serverLocalAddr;
		serverLocalAddr.Clear();
		serverLocalAddr.m_port = port;
		m_hListenSock = GetSteamInterface().CreateListenSocketIP(serverLocalAddr);

		if(m_hListenSock == k_HSteamListenSocket_Invalid)
		{
			outErr = {pragma::networking::ErrorCode::UnableToListenOnPort,"Failed to listen on port " +std::to_string(serverLocalAddr.m_port) +"!"};
			return false;
		}
	}
#ifdef USE_STEAMWORKS_NETWORKING
	else
	{
		m_hListenSock = GetSteamInterface().CreateListenSocketP2P(0);
		if(m_hListenSock == k_HSteamListenSocket_Invalid)
		{
			outErr = {pragma::networking::ErrorCode::UnableToListenOnPort,"Failed to create P2P listener!"};
			return false;
		}
		m_steamId = SteamUser()->GetSteamID();
	}
#endif
	return true;
}
bool pragma::networking::SteamServer::DoShutdown(pragma::networking::Error &outErr)
{
	return true;
}
bool pragma::networking::SteamServer::Heartbeat()
{
	return true; // TODO
}
std::optional<std::string> pragma::networking::SteamServer::GetHostIP() const
{
	return {}; // TODO
}
std::optional<pragma::networking::Port> pragma::networking::SteamServer::GetHostPort() const {return {};}
std::optional<uint64_t> pragma::networking::SteamServer::GetSteamId() const {return m_steamId.has_value() ? m_steamId->ConvertToUint64() : std::optional<uint64_t>{};}
bool pragma::networking::SteamServer::IsPeerToPeer() const {return m_steamId.has_value();}
bool pragma::networking::SteamServer::PollMessages(pragma::networking::Error &outErr)
{
	std::array<ISteamNetworkingMessage*,256> incommingMessages;
	auto numMsgs = GetSteamInterface().ReceiveMessagesOnListenSocket(m_hListenSock,incommingMessages.data(),incommingMessages.size());
	if(numMsgs < 0)
		return false;
	if(numMsgs == 0)
		return true;
	for(auto i=decltype(numMsgs){0u};i<numMsgs;++i)
	{
		auto *pIncomingMsg = incommingMessages.at(i);
		auto itClient = m_conHandleToClient.find(pIncomingMsg->m_conn);
		if(itClient == m_conHandleToClient.end())
			outErr = {pragma::networking::ErrorCode::InvalidClient,"Received packet for unknown client!"};
		else
		{
			auto &client = *itClient->second;
			auto packet = client.ReceiveDataFragment(*this,*pIncomingMsg);
			if(packet.has_value())
				HandlePacket(client,*packet);
		}
		pIncomingMsg->Release();
	}
	return true;
}
bool pragma::networking::SteamServer::PollEvents(pragma::networking::Error &outErr)
{
	auto success = PollMessages(outErr);
#ifndef USE_STEAMWORKS_NETWORKING
	GetSteamInterface().RunCallbacks(this);
#endif
	return success;
}
void pragma::networking::SteamServer::SetTimeoutDuration(float duration) {}

ISteamNetworkingSockets &pragma::networking::SteamServer::GetInterface() const {return GetSteamInterface();}

void pragma::networking::SteamServer::OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo)
{
	// TODO: Check if this is called asynchronously
	m_statusError = {};
	switch(pInfo->m_info.m_eState)
	{
	case k_ESteamNetworkingConnectionState_None:
		break;
	case k_ESteamNetworkingConnectionState_Connecting:
	{
		auto result = GetSteamInterface().AcceptConnection(pInfo->m_hConn);
		if(result != k_EResultOK)
		{
			if(result == k_EResultInvalidParam)
			{
				// Note: For some reason this callback is called twice for every new connection.
				// When trying to accept the connection during the first execution, it always fails with
				// 'k_EResultInvalidParam', cause unknown. We mustn't drop the client in this case,
				// because the second execution works fine.
				// TODO: Find out what's going on here and fix this!
				break;
			};
			GetSteamInterface().CloseConnection(pInfo->m_hConn,0,nullptr,false);
			m_statusError = {pragma::networking::ErrorCode::UnableToAcceptClient,"Client could not be accepted!",result};
			break;
		}
		auto itClient = m_conHandleToClient.find(pInfo->m_hConn);
		if(itClient == m_conHandleToClient.end())
		{
			// New client
			auto cl = AddClient<pragma::networking::SteamServerClient>(*this,pInfo->m_hConn);
			m_conHandleToClient[pInfo->m_hConn] = cl.get();
		}
		break;
	}
	case k_ESteamNetworkingConnectionState_FindingRoute:
		break;
	case k_ESteamNetworkingConnectionState_Connected:
		break;
	case k_ESteamNetworkingConnectionState_ClosedByPeer:
	case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
	{
		GetSteamInterface().CloseConnection(pInfo->m_hConn,0,nullptr,false);
		auto itCl = m_conHandleToClient.find(pInfo->m_hConn);
		if(itCl == m_conHandleToClient.end())
		{
			m_statusError = {pragma::networking::ErrorCode::InvalidClient,"Connection status of invalid client has changed!"};
			break;
		}
		m_conHandleToClient.erase(itCl);
		pragma::networking::Error err;
		DropClient(
			*itCl->second,
			(pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer) ? pragma::networking::DropReason::Disconnected : pragma::networking::DropReason::Error,
			err
		);
		break;
	}
	case k_ESteamNetworkingConnectionState_FinWait:
		break;
	case k_ESteamNetworkingConnectionState_Linger:
		break;
	case k_ESteamNetworkingConnectionState_Dead:
		break;
	}
}

#ifndef USE_STEAMWORKS_NETWORKING
void pragma::networking::SteamServer::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo)
{
	OnConnectionStatusChanged(pInfo);
}
#endif

ISteamNetworkingSockets &pragma::networking::SteamServerClient::GetInterface() const
{
	return m_server.GetInterface();
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
	PRAGMA_EXPORT void initialize_game_server(NetworkState &nw,std::unique_ptr<pragma::networking::IServer> &outServer,pragma::networking::Error &outErr)
	{
		outServer = std::make_unique<pragma::networking::SteamServer>();
	}
};
