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
m_lastStatusMessage(),
m_peers{},
m_pendingJoinRequests{},
m_pendingPlayerInputs{}
{
	// no-op
}


bool NetHost::start(std::uint16_t port)
{
	m_lastStatusMessage.clear();
	m_pendingJoinRequests.clear();
	m_pendingPlayerInputs.clear();
	
	m_socket.unbind();
	
	if(m_socket.bind(port) != sf::Socket::Done)
		return false;
	
	m_socket.setBlocking(false);
	
	for(auto& peer : m_peers)
		peer = RemotePeer{};
	
	return true;
}


void NetHost::stop()
{
	m_lastStatusMessage.clear();
	m_pendingJoinRequests.clear();
	m_pendingPlayerInputs.clear();
	
	m_socket.unbind();
	
	for(auto& peer : m_peers)
		peer = RemotePeer{};
}


void NetHost::pumpIncomingPackets()
{
	for(;;)
	{
		std::uint8_t buffer[2048];
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
		
		switch(header.packetType)
		{
			case PacketType::JoinRequest:
			{
				if(received != sizeof(JoinRequestPacket))
					continue;
				
				JoinRequestPacket packet{};
				std::memcpy(&packet, buffer, sizeof(JoinRequestPacket));
				handleJoinRequestPacket(packet, sender, static_cast<std::uint16_t>(senderPort));
				break;
			}
			
			case PacketType::PlayerInput:
			{
				if(received != sizeof(PlayerInputPacket))
					continue;
				
				PlayerInputPacket packet{};
				std::memcpy(&packet, buffer, sizeof(PlayerInputPacket));
				handlePlayerInputPacket(packet, sender, static_cast<std::uint16_t>(senderPort));
				break;
			}
			
			default:
				break;
		}
	}
}


bool NetHost::pollJoinRequest(sf::IpAddress& outAddress, std::uint16_t& outPort)
{
	if(m_pendingJoinRequests.empty())
		return false;
	
	const PendingJoinRequest request = m_pendingJoinRequests.front();
	m_pendingJoinRequests.pop_front();
	
	outAddress = request.address;
	outPort = request.port;
	
	return true;
}


bool NetHost::sendJoinAccept(const sf::IpAddress& address, std::uint16_t port, const JoinAcceptPacket& packet)
{
	return m_socket.send(&packet, sizeof(packet), address, port) == sf::Socket::Done;
}


bool NetHost::pollPlayerInput(PlayerInputPacket& outPacket)
{
	if(m_pendingPlayerInputs.empty())
		return false;
	
	outPacket = m_pendingPlayerInputs.front();
	m_pendingPlayerInputs.pop_front();
	
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
		
		m_lastStatusMessage = "Client connected: " + address.toString() +
		":" + std::to_string(port);
		
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


void NetHost::handleJoinRequestPacket(const JoinRequestPacket& packet,
									  const sf::IpAddress& sender,
									  std::uint16_t senderPort)
{
	(void)packet;
	
	if(findPeerByEndpoint(sender, senderPort) != nullptr)
		return;
	
	if(hasPendingJoinRequest(sender, senderPort))
		return;
	
	PendingJoinRequest request{};
	request.address = sender;
	request.port = senderPort;
	m_pendingJoinRequests.push_back(request);
}


void NetHost::handlePlayerInputPacket(const PlayerInputPacket& packet,
									  const sf::IpAddress& sender,
									  std::uint16_t senderPort)
{
	RemotePeer* peer = findPeerByEndpoint(sender, senderPort);
	if(peer == nullptr)
		return;
	
	if(packet.peerId != peer->peerId)
		return;
	
	m_pendingPlayerInputs.push_back(packet);
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


bool NetHost::hasPendingJoinRequest(const sf::IpAddress& address, std::uint16_t port) const
{
	for(const PendingJoinRequest& request : m_pendingJoinRequests)
	{
		if(request.address == address && request.port == port)
			return true;
	}
	
	return false;
}


const std::string& NetHost::getLastStatusMessage() const
{
	return m_lastStatusMessage;
}


void NetHost::clearLastStatusMessage()
{
	m_lastStatusMessage.clear();
}
