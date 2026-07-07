/*
 * NetClient.hpp
 *
 * 06-07-2026 by madpl
 */
#pragma once

#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/UdpSocket.hpp>
#include <cstdint>
#include <string>
#include <network/NetworkPackets.hpp>


class NetClient
{
public:
	NetClient();
	
	bool connectTo(const sf::IpAddress& host, std::uint16_t port);
	void disconnect();
	
	bool sendJoinRequest();
	bool pollJoinAccept(JoinAcceptPacket& outPacket);
	
	bool sendPlayerInput(const PlayerInputPacket& packet);
	bool pollWorldState(WorldStatePacket& outPacket);
	
	bool isConnected() const;
	bool isAccepted() const;
	
	std::uint32_t getAssignedPeerId() const;
	std::int32_t getAssignedSlotIndex() const;
	
	const std::string& getLastStatusMessage() const;
	void clearLastStatusMessage();
	
private:
	sf::UdpSocket m_socket;
	sf::IpAddress m_hostAddress;
	std::uint16_t m_hostPort;
	std::uint32_t m_assignedPeerId;
	std::int32_t m_assignedSlotIndex;
	std::string m_lastStatusMessage;
	bool m_connected;
	bool m_accepted;
};
