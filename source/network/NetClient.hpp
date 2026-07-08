/*
 * NetClient.hpp
 *
 * 06-07-2026 by madpl
 */
#pragma once

#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/UdpSocket.hpp>
#include <cstdint>
#include <deque>
#include <string>
#include <network/NetworkPackets.hpp>


class NetClient
{
public:
	NetClient();
	
	bool connectTo(const sf::IpAddress& hostAddress, std::uint16_t hostPort);
	void disconnect();
	
	bool sendJoinRequest();
	bool sendPlayerInput(const PlayerInputPacket& packet);
	
	void pumpIncomingPackets();
	
	bool pollJoinAccept(JoinAcceptPacket& outPacket);
	bool pollWorldState(WorldStatePacket& outPacket);
	
	bool isAccepted() const;
	std::uint32_t getAssignedPeerId() const;
	std::int32_t getAssignedSlotIndex() const;
	
	const std::string& getLastStatusMessage() const;
	void clearLastStatusMessage();
	
private:
	void handleJoinAcceptPacket(const JoinAcceptPacket& packet,
								const sf::IpAddress& sender,
							 std::uint16_t senderPort);
	
	void handleWorldStatePacket(const WorldStatePacket& packet,
								const sf::IpAddress& sender,
							 std::uint16_t senderPort);
	
	bool isFromConfiguredHost(const sf::IpAddress& sender, std::uint16_t senderPort) const;
	
private:
	sf::UdpSocket m_socket;
	sf::IpAddress m_hostAddress;
	std::uint16_t m_hostPort;
	bool m_connected;
	bool m_accepted;
	std::uint32_t m_assignedPeerId;
	std::int32_t m_assignedSlotIndex;
	std::string m_lastStatusMessage;
	
	std::deque<JoinAcceptPacket> m_pendingJoinAccepts;
	std::deque<WorldStatePacket> m_pendingWorldStates;
};
