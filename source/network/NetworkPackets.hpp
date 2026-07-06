/*
 * NetworkPackets.hpp
 *
 * 06-07-2026 by madpl
 */
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>


namespace NetGame
{
	static constexpr std::uint32_t ProtocolVersion = 1;
	static constexpr std::size_t MaxPlayers = 4;
	static constexpr std::uint32_t HostPeerId = 0;
	static constexpr std::int32_t InvalidSlotIndex = -1;
}


enum class PacketType : std::uint8_t
{
	Invalid = 0,
	JoinRequest = 1,
	JoinAccept = 2,
	PlayerInput = 3,
	WorldState = 4,
	DisconnectNotice = 5
};


enum class SlotOccupancy : std::uint8_t
{
	Empty = 0,
	Occupied = 1
};


struct PacketHeader
{
	std::uint32_t protocolVersion{NetGame::ProtocolVersion};
	PacketType packetType{PacketType::Invalid};
	std::uint16_t payloadSize{0};
};


struct JoinRequestPacket
{
	PacketHeader header
	{
		NetGame::ProtocolVersion,
		PacketType::JoinRequest,
		static_cast<std::uint16_t>(sizeof(JoinRequestPacket) - sizeof(PacketHeader))
	};
	
	std::uint32_t requestedTickRate{60};
};


struct JoinAcceptPacket
{
	PacketHeader header
	{
		NetGame::ProtocolVersion,
		PacketType::JoinAccept,
		static_cast<std::uint16_t>(sizeof(JoinAcceptPacket) - sizeof(PacketHeader))
	};
	
	std::uint8_t accepted{0};
	std::uint8_t reserved0{0};
	std::uint16_t reserved1{0};
	
	std::uint32_t assignedPeerId{0};
	std::int32_t assignedSlotIndex{NetGame::InvalidSlotIndex};
};


struct PlayerInputPacket
{
	PacketHeader header
	{
		NetGame::ProtocolVersion,
		PacketType::PlayerInput,
		static_cast<std::uint16_t>(sizeof(PlayerInputPacket) - sizeof(PacketHeader))
	};
	
	std::uint32_t peerId{0};
	std::uint32_t tick{0};
	
	float collective{0.0f};
	float cyclicPitch{0.0f};
	float cyclicRoll{0.0f};
	float yawPedal{0.0f};
	
	std::uint8_t brake{0};
	std::uint8_t fireCannon{0};
	std::uint8_t launchMissile{0};
	std::uint8_t pauseRequested{0};
};


struct HelicopterSlotStatePacket
{
	std::uint8_t slotIndex{0};
	std::uint8_t occupied{0};
	std::uint16_t reserved0{0};
	
	std::uint32_t ownerPeerId{0};
	std::uint32_t lastProcessedTick{0};
	
	float x{0.0f};
	float y{0.0f};
	float z{0.0f};
	
	float yawDegrees{0.0f};
	float pitchDegrees{0.0f};
	float rollDegrees{0.0f};
	
	float speedMetersPerSecond{0.0f};
	float verticalSpeedMetersPerSecond{0.0f};
	float altitudeAboveGroundMeters{0.0f};
};


struct WorldStatePacket
{
	PacketHeader header
	{
		NetGame::ProtocolVersion,
		PacketType::WorldState,
		static_cast<std::uint16_t>(sizeof(WorldStatePacket) - sizeof(PacketHeader))
	};
	
	std::uint32_t serverTick{0};
	std::uint32_t yourPeerId{0};
	std::int32_t yourSlotIndex{NetGame::InvalidSlotIndex};
	std::uint32_t activePlayerCount{0};
	
	std::array<HelicopterSlotStatePacket, NetGame::MaxPlayers> slots{};
};


struct DisconnectNoticePacket
{
	PacketHeader header
	{
		NetGame::ProtocolVersion,
		PacketType::DisconnectNotice,
		static_cast<std::uint16_t>(sizeof(DisconnectNoticePacket) - sizeof(PacketHeader))
	};
	
	std::uint32_t peerId{0};
	std::int32_t slotIndex{NetGame::InvalidSlotIndex};
};


static_assert(std::is_trivially_copyable_v<PacketHeader>);
static_assert(std::is_trivially_copyable_v<JoinRequestPacket>);
static_assert(std::is_trivially_copyable_v<JoinAcceptPacket>);
static_assert(std::is_trivially_copyable_v<PlayerInputPacket>);
static_assert(std::is_trivially_copyable_v<HelicopterSlotStatePacket>);
static_assert(std::is_trivially_copyable_v<WorldStatePacket>);
static_assert(std::is_trivially_copyable_v<DisconnectNoticePacket>);
