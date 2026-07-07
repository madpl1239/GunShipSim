/*
 * NetHost.hpp
 *
 * 06-07-2026 by madpl
 */
#pragma once

#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/UdpSocket.hpp>
#include <array>
#include <cstdint>
#include <string>
#include <network/NetworkPackets.hpp>


class NetHost
{
public:
	struct RemotePeer
	{
		bool active{false};
		std::uint32_t peerId{0};
		std::int32_t slotIndex{NetGame::InvalidSlotIndex};
		sf::IpAddress address{sf::IpAddress::None};
		std::uint16_t port{0};
	};
	
	NetHost();
	
	bool start(std::uint16_t port);
	void stop();
	
	bool pollJoinRequest(sf::IpAddress& outAddress, std::uint16_t& outPort);
	bool sendJoinAccept(const sf::IpAddress& address, std::uint16_t port, const JoinAcceptPacket& packet);
	
	bool pollPlayerInput(PlayerInputPacket& outPacket);
	bool sendWorldStateToAll(const WorldStatePacket& packet);
	
	bool registerPeer(const sf::IpAddress& address, std::uint16_t port, std::uint32_t peerId, std::int32_t slotIndex);
	void unregisterPeer(std::uint32_t peerId);
	
	const std::string& getLastStatusMessage() const;
	void clearLastStatusMessage();
	
	const std::array<RemotePeer, NetGame::MaxPlayers>& getPeers() const;
	
private:
	RemotePeer* findPeerByEndpoint(const sf::IpAddress& address, std::uint16_t port);
	RemotePeer* findPeerByPeerId(std::uint32_t peerId);
	
private:
	sf::UdpSocket m_socket;
	std::string m_lastStatusMessage;
	std::array<RemotePeer, NetGame::MaxPlayers> m_peers;
};
