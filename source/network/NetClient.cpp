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
m_assignedPeerId(0),
m_assignedSlotIndex(NetGame::InvalidSlotIndex),
m_lastStatusMessage(),
m_connected(false),
m_accepted(false)
{
	// empty
}


bool NetClient::connectTo(const sf::IpAddress& host, std::uint16_t port)
{
	m_socket.unbind();
	
	if(m_socket.bind(sf::Socket::AnyPort) != sf::Socket::Done)
		return false;
	
	m_socket.setBlocking(false);
	m_hostAddress = host;
	m_hostPort = port;
	m_connected = true;
	m_accepted = false;
	m_assignedPeerId = 0;
	m_assignedSlotIndex = NetGame::InvalidSlotIndex;
	
	m_lastStatusMessage = "Connecting to host...";
	
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
	m_lastStatusMessage.clear();
}


bool NetClient::sendJoinRequest()
{
	if(not m_connected)
		return false;
	
	JoinRequestPacket packet{};
	
	return m_socket.send(&packet, sizeof(packet), m_hostAddress, m_hostPort) == sf::Socket::Done;
}


bool NetClient::pollJoinAccept(JoinAcceptPacket& outPacket)
{
	std::uint8_t buffer[256];
	std::size_t received = 0;
	sf::IpAddress sender;
	unsigned short senderPort = 0;
	
	const sf::Socket::Status status =
	m_socket.receive(buffer, sizeof(buffer), received, sender, senderPort);
	
	if(status != sf::Socket::Done)
		return false;
	
	if(sender != m_hostAddress || senderPort != m_hostPort)
		return false;
	
	if(received != sizeof(JoinAcceptPacket))
		return false;
	
	JoinAcceptPacket packet{};
	std::memcpy(&packet, buffer, sizeof(JoinAcceptPacket));
	
	if(packet.header.protocolVersion != NetGame::ProtocolVersion)
		return false;
	
	if(packet.header.packetType != PacketType::JoinAccept)
		return false;
	
	outPacket = packet;
	
	if(packet.accepted != 0)
	{
		m_accepted = true;
		m_assignedPeerId = packet.assignedPeerId;
		m_assignedSlotIndex = packet.assignedSlotIndex;
		m_lastStatusMessage = "Connected to host";
	}
	
	return true;
}


bool NetClient::sendPlayerInput(const PlayerInputPacket& packet)
{
	if(not m_connected || not m_accepted)
		return false;
	
	return m_socket.send(&packet, sizeof(packet), m_hostAddress, m_hostPort) == sf::Socket::Done;
}


bool NetClient::pollWorldState(WorldStatePacket& outPacket)
{
	std::uint8_t buffer[2048];
	std::size_t received = 0;
	sf::IpAddress sender;
	unsigned short senderPort = 0;
	
	const sf::Socket::Status status =
	m_socket.receive(buffer, sizeof(buffer), received, sender, senderPort);
	
	if(status != sf::Socket::Done)
		return false;
	
	if(sender != m_hostAddress || senderPort != m_hostPort)
		return false;
	
	if(received != sizeof(WorldStatePacket))
		return false;
	
	WorldStatePacket packet{};
	std::memcpy(&packet, buffer, sizeof(WorldStatePacket));
	
	if(packet.header.protocolVersion != NetGame::ProtocolVersion)
		return false;
	
	if(packet.header.packetType != PacketType::WorldState)
		return false;
	
	outPacket = packet;
	
	return true;
}


bool NetClient::isConnected() const
{
	return m_connected;
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
