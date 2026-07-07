/*
 * NetHost.cpp
 *
 * 06-07-2026 by madpl
 */
#include <SFML/Network/Socket.hpp>
#include <cstring>
#include <network/NetHost.hpp>


NetHost::NetHost():
	m_socket(),
	m_peers{}
{
	// no-op
}


bool NetHost::start(std::uint16_t port)
{
	if(m_socket.bind(port) != sf::Socket::Done)
		return false;
	
	m_socket.setBlocking(false);
	
	for(auto& peer : m_peers)
		peer = RemotePeer{};
	
	return true;
}


void NetHost::stop()
{
	m_socket.unbind();
	
	for(auto& peer : m_peers)
		peer = RemotePeer{};
}


bool NetHost::pollJoinRequest(sf::IpAddress& outAddress, std::uint16_t& outPort)
{
	std::uint8_t buffer[256];
	std::size_t received = 0;
	sf::IpAddress sender;
	unsigned short senderPort = 0;
	
	const sf::Socket::Status status =
	m_socket.receive(buffer, sizeof(buffer), received, sender, senderPort);
	
	if(status != sf::Socket::Done)
		return false;
	
	if(received != sizeof(JoinRequestPacket))
		return false;
	
	JoinRequestPacket packet{};
	std::memcpy(&packet, buffer, sizeof(JoinRequestPacket));
	
	if(packet.header.protocolVersion != NetGame::ProtocolVersion)
		return false;
	
	if(packet.header.packetType != PacketType::JoinRequest)
		return false;
	
	outAddress = sender;
	outPort = static_cast<std::uint16_t>(senderPort);
	
	return true;
}


bool NetHost::sendJoinAccept(const sf::IpAddress& address, std::uint16_t port, const JoinAcceptPacket& packet)
{
	return m_socket.send(&packet, sizeof(packet), address, port) == sf::Socket::Done;
}


bool NetHost::pollPlayerInput(PlayerInputPacket& outPacket)
{
	std::uint8_t buffer[256];
	std::size_t received = 0;
	sf::IpAddress sender;
	unsigned short senderPort = 0;
	
	const sf::Socket::Status status =
	m_socket.receive(buffer, sizeof(buffer), received, sender, senderPort);
	
	if(status != sf::Socket::Done)
		return false;
	
	if(received != sizeof(PlayerInputPacket))
		return false;
	
	PlayerInputPacket packet{};
	std::memcpy(&packet, buffer, sizeof(PlayerInputPacket));
	
	if(packet.header.protocolVersion != NetGame::ProtocolVersion)
		return false;
	
	if(packet.header.packetType != PacketType::PlayerInput)
		return false;
	
	RemotePeer* peer = findPeerByEndpoint(sender, static_cast<std::uint16_t>(senderPort));
	if(peer == nullptr)
		return false;
	
	if(packet.peerId != peer->peerId)
		return false;
	
	outPacket = packet;
	
	return true;
}


bool NetHost::sendWorldStateToAll(const WorldStatePacket& packet)
{
	bool allSent = true;
	
	for(const RemotePeer& peer : m_peers)
	{
		if(not peer.active)
			continue;
		
		if(m_socket.send(&packet, sizeof(packet), peer.address, peer.port) != sf::Socket::Done)
			allSent = false;
	}
	
	return allSent;
}


bool NetHost::registerPeer(const sf::IpAddress& address, std::uint16_t port,
						   std::uint32_t peerId, std::int32_t slotIndex)
{
	for(auto& peer : m_peers)
	{
		if(peer.active)
			continue;
		
		peer.active = true;
		peer.peerId = peerId;
		peer.slotIndex = slotIndex;
		peer.address = address;
		peer.port = port;
		
		return true;
	}
	
	return false;
}


void NetHost::unregisterPeer(std::uint32_t peerId)
{
	RemotePeer* peer = findPeerByPeerId(peerId);
	if(peer != nullptr)
		*peer = RemotePeer{};
}


const std::array<NetHost::RemotePeer, NetGame::MaxPlayers>& NetHost::getPeers() const
{
	return m_peers;
}


NetHost::RemotePeer* NetHost::findPeerByEndpoint(const sf::IpAddress& address, std::uint16_t port)
{
	for(auto& peer : m_peers)
	{
		if(peer.active && peer.address == address && peer.port == port)
			return &peer;
	}
	
	return nullptr;
}


NetHost::RemotePeer* NetHost::findPeerByPeerId(std::uint32_t peerId)
{
	for(auto& peer : m_peers)
	{
		if(peer.active && peer.peerId == peerId)
			return &peer;
	}
	
	return nullptr;
}
