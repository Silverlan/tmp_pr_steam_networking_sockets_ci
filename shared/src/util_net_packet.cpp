#include "pr_steam_networking/util_net_packet.hpp"

std::optional<NetPacket> NetPacketReceiver::ReceiveDataFragment(BaseSteamNetworkingSocket &sns, ISteamNetworkingMessage &msg)
{
	if(m_bReceivedHeader == false) {
		auto size = msg.GetSize();
		if(size != sizeof(m_packetHeader))
			return {};
		auto *data = msg.GetData();
		memcpy(&m_packetHeader, data, size);
		m_bReceivedHeader = true;
		return {};
	}
	// Receive body
	auto size = msg.GetSize();
	auto *data = msg.GetData();
	m_bReceivedHeader = false;
	m_packetBody = NetPacket {m_packetHeader.messageId, size};
	memcpy(m_packetBody->GetData(), data, size);

	m_packetBody.SetTimeActivated(util::clock::to_int(util::clock::get_duration_since_start()));
	return m_packetBody;
}

bool NetPacketDispatcher::SendPacket(ISteamNetworkingSockets &sns, uint32_t hConnection, pragma::networking::Protocol protocol, NetPacket &packet, pragma::networking::Error &outErr)
{
	auto bodySize = packet->GetSize();
	HeaderData headerData {packet.GetMessageID(), bodySize};
	return SendData(sns, hConnection, protocol, &headerData, sizeof(headerData), outErr) && SendData(sns, hConnection, protocol, packet->GetData(), bodySize, outErr);
}
bool NetPacketDispatcher::SendData(ISteamNetworkingSockets &sns, uint32_t hConnection, pragma::networking::Protocol protocol, const void *data, uint32_t dataSize, pragma::networking::Error &outErr)
{
	auto sendFlags = get_send_flags(protocol);
	auto eResult = sns.SendMessageToConnection(hConnection, data, dataSize, sendFlags);
	switch(eResult) {
	case k_EResultOK:
		return true;
	case k_EResultInvalidParam:
		if(!hConnection)
			outErr = {pragma::networking::ErrorCode::InvalidConnectionHandle, "Invalid connection handle!"};
		else
			outErr = {pragma::networking::ErrorCode::MessageTooLarge, "Message is too big!"};
		break;
	case k_EResultInvalidState:
		outErr = {pragma::networking::ErrorCode::InvalidConnectionHandle, "Connection is in an invalid state!"};
		break;
	case k_EResultNoConnection:
		outErr = {pragma::networking::ErrorCode::InvalidConnectionHandle, "Connection has ended!"};
		break;
	case k_EResultIgnored:
		outErr = {pragma::networking::ErrorCode::GenericError, "Not ready to send message!"};
		break;
	case k_EResultLimitExceeded:
		outErr = {pragma::networking::ErrorCode::UnableToSendPacket, "Too much data on send queue!"};
		break;
	default:
		outErr = {pragma::networking::ErrorCode::UnableToSendPacket, "Unknown error!"};
		break;
	}
	return false;
}
