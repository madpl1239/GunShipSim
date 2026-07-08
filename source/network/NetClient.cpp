/*
 * NetClient.cpp
 *
 * 06-07-2026 by madpl
 */
#include <SFML/Network/Socket.hpp>
#include <cstring>
#include <network/NetClient.hpp>


NetClient::NetClient():
m_socket(),
m_hostAddress(sf::IpAddress::None),
m_hostPort(0),
m_connected(false),
m_accepted(false),
m_assignedPeerId(0),
m_assignedSlotIndex(NetGame::InvalidSlotIndex),
m_lastStatusMessage(),
m_pendingJoinAccepts{},
m_pendingWorldStates{}
{
}


bool NetClient::connectTo(const sf::IpAddress& hostAddress, std::uint16_t hostPort)
{
	disconnect();
	
	m_hostAddress = hostAddress;
	m_hostPort = hostPort;
	
	if(m_socket.bind(sf::Socket::AnyPort) != sf::Socket::Done)
	{
		m_lastStatusMessage = "Failed to bind client socket";
		return false;
	}
	
	m_socket.setBlocking(false);
	
	m_connected = true;
	m_accepted = false;
	m_assignedPeerId = 0;
	m_assignedSlotIndex = NetGame::InvalidSlotIndex;
	m_pendingJoinAccepts.clear();
	m_pendingWorldStates.clear();
	m_lastStatusMessage = "Client socket ready";
	
	return true;
}


void NetClient::disconnect()
{
	m_socket.unbind();
	
	m_hostAddress = sf::IpAddress::None;
	m_hostPort = 0;
	m_connected = false;
	m_accepted = false;
	m_assignedPeerId = 0;
	m_assignedSlotIndex = NetGame::InvalidSlotIndex;
	m_pendingJoinAccepts.clear();
	m_pendingWorldStates.clear();
	m_lastStatusMessage.clear();
}


bool NetClient::sendJoinRequest()
{
	if(not m_connected)
		return false;
	
	JoinRequestPacket packet{};
	packet.header.protocolVersion = NetGame::ProtocolVersion;
	packet.header.packetType = PacketType::JoinRequest;
	
	const sf::Socket::Status status =
	m_socket.send(&packet, sizeof(packet), m_hostAddress, m_hostPort);
	
	if(status == sf::Socket::Done)
	{
		m_lastStatusMessage = "Join request sent";
		return true;
	}
	
	m_lastStatusMessage = "Failed to send join request";
	return false;
}


bool NetClient::sendPlayerInput(const PlayerInputPacket& packet)
{
	if(not m_connected)
		return false;
	
	const sf::Socket::Status status =
	m_socket.send(&packet, sizeof(packet), m_hostAddress, m_hostPort);
	
	return status == sf::Socket::Done;
}


void NetClient::pumpIncomingPackets()
{
	for(;;)
	{
		std::uint8_t buffer[4096];
		std::size_t received = 0;
		sf::IpAddress sender;
		unsigned short senderPort = 0;
		
		const sf::Socket::Status status =
		m_socket.receive(buffer, sizeof(buffer), received, sender, senderPort);
		
		if(status == sf::Socket::NotReady)
			break;
		
		if(status != sf::Socket::Done)
			break;
		
		if(received < sizeof(PacketHeader))
			continue;
		
		PacketHeader header{};
		std::memcpy(&header, buffer, sizeof(PacketHeader));
		
		if(header.protocolVersion != NetGame::ProtocolVersion)
			continue;
		
		if(header.packetType == PacketType::JoinAccept)
		{
			if(received != sizeof(JoinAcceptPacket))
				continue;
			
			JoinAcceptPacket packet{};
			std::memcpy(&packet, buffer, sizeof(JoinAcceptPacket));
			handleJoinAcceptPacket(packet, sender, static_cast<std::uint16_t>(senderPort));
		}
		else if(header.packetType == PacketType::WorldState)
		{
			if(received != sizeof(WorldStatePacket))
				continue;
			
			WorldStatePacket packet{};
			std::memcpy(&packet, buffer, sizeof(WorldStatePacket));
			handleWorldStatePacket(packet, sender, static_cast<std::uint16_t>(senderPort));
		}
	}
}


bool NetClient::pollJoinAccept(JoinAcceptPacket& outPacket)
{
	if(m_pendingJoinAccepts.empty())
		return false;
	
	outPacket = m_pendingJoinAccepts.front();
	m_pendingJoinAccepts.pop_front();
	return true;
}


bool NetClient::pollWorldState(WorldStatePacket& outPacket)
{
	if(m_pendingWorldStates.empty())
		return false;
	
	outPacket = m_pendingWorldStates.front();
	m_pendingWorldStates.pop_front();
	return true;
}


bool NetClient::isAccepted() const
{
	return m_accepted;
}


std::uint32_t NetClient::getAssignedPeerId() const
{
	return m_assignedPeerId;
}


std::int32_t NetClient::getAssignedSlotIndex() const
{
	return m_assignedSlotIndex;
}


const std::string& NetClient::getLastStatusMessage() const
{
	return m_lastStatusMessage;
}


void NetClient::clearLastStatusMessage()
{
	m_lastStatusMessage.clear();
}


void NetClient::handleJoinAcceptPacket(const JoinAcceptPacket& packet,
									   const sf::IpAddress& sender,
									   std::uint16_t senderPort)
{
	if(not isFromConfiguredHost(sender, senderPort))
		return;
	
	m_pendingJoinAccepts.push_back(packet);
	
	if(packet.accepted != 0)
	{
		m_accepted = true;
		m_assignedPeerId = packet.assignedPeerId;
		m_assignedSlotIndex = packet.assignedSlotIndex;
		m_lastStatusMessage = "Join accepted";
	}
	else
	{
		m_lastStatusMessage = "Join rejected";
	}
}


void NetClient::handleWorldStatePacket(const WorldStatePacket& packet,
									   const sf::IpAddress& sender,
									   std::uint16_t senderPort)
{
	if(not isFromConfiguredHost(sender, senderPort))
		return;
	
	m_pendingWorldStates.push_back(packet);
}


bool NetClient::isFromConfiguredHost(const sf::IpAddress& sender, std::uint16_t senderPort) const
{
	return sender == m_hostAddress && senderPort == m_hostPort;
}
