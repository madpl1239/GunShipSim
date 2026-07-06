/*
 * NetHost.hpp
 * 
 * 06-07-2026 by madpl
 */
#pragma once

#include <cstdint>
#include <optional>
#include <SFML/Network/UdpSocket.hpp>
#include <network/NetPacket.hpp>


class NetHost
{
public:
	bool start(std::uint16_t port);
	void stop();
	
	bool pollInputSnapshot(InputSnapshotPacket& outPacket);
	bool sendStateSnapshot(const StateSnapshotPacket& packet);
	
private:
	sf::UdpSocket m_socket;
	sf::IpAddress m_clientAddress;
	std::uint16_t m_clientPort = 0;
	bool m_hasClient = false;
};
