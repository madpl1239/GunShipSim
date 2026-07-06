#pragma once

#include <cstdint>
#include <SFML/Network/UdpSocket.hpp>
#include "NetPacket.hpp"


class NetClient
{
public:
	bool connectTo(const sf::IpAddress& host, std::uint16_t port);
	void disconnect();
	
	bool sendInputSnapshot(const InputSnapshotPacket& packet);
	bool pollStateSnapshot(StateSnapshotPacket& outPacket);
	
private:
	sf::UdpSocket m_socket;
	sf::IpAddress m_hostAddress;
	std::uint16_t m_hostPort = 0;
	bool m_connected = false;
};
