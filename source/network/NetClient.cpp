#include "NetClient.hpp"
#include "NetCodec.hpp"


bool NetClient::connectTo(const sf::IpAddress& host, std::uint16_t port)
{
	if(m_socket.bind(sf::Socket::AnyPort) != sf::Socket::Done)
		return false;
	
	m_socket.setBlocking(false);
	m_hostAddress = host;
	m_hostPort = port;
	m_connected = true;
	return true;
}


void NetClient::disconnect()
{
	m_socket.unbind();
	m_connected = false;
}


bool NetClient::sendInputSnapshot(const InputSnapshotPacket& packet)
{
	if(!m_connected)
		return false;
	
	auto bytes = NetCodec::encode(packet);
	return m_socket.send(bytes.data(), bytes.size(), m_hostAddress, m_hostPort) == sf::Socket::Done;
}


bool NetClient::pollStateSnapshot(StateSnapshotPacket& outPacket)
{
	std::uint8_t buffer[128];
	std::size_t received = 0;
	sf::IpAddress sender;
	unsigned short senderPort = 0;
	
	auto status = m_socket.receive(buffer, sizeof(buffer), received, sender, senderPort);
	if(status != sf::Socket::Done)
		return false;
	
	return NetCodec::decode(buffer, received, outPacket);
}
