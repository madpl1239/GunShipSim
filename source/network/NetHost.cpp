#include "NetHost.hpp"
#include "NetCodec.hpp"


bool NetHost::start(std::uint16_t port)
{
	if(m_socket.bind(port) != sf::Socket::Done)
		return false;
	
	m_socket.setBlocking(false);
	return true;
}


void NetHost::stop()
{
	m_socket.unbind();
	m_hasClient = false;
	m_clientPort = 0;
}


bool NetHost::pollInputSnapshot(InputSnapshotPacket& outPacket)
{
	std::uint8_t buffer[128];
	std::size_t received = 0;
	sf::IpAddress sender;
	unsigned short senderPort = 0;
	
	auto status = m_socket.receive(buffer, sizeof(buffer), received, sender, senderPort);
	if(status != sf::Socket::Done)
		return false;
	
	if(!NetCodec::decode(buffer, received, outPacket))
		return false;
	
	m_clientAddress = sender;
	m_clientPort = senderPort;
	m_hasClient = true;
	return true;
}


bool NetHost::sendStateSnapshot(const StateSnapshotPacket& packet)
{
	if(!m_hasClient)
		return false;
	
	auto bytes = NetCodec::encode(packet);
	return m_socket.send(bytes.data(), bytes.size(), m_clientAddress, m_clientPort) == sf::Socket::Done;
}
