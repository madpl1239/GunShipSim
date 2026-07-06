/*
 * HelicopterSlot.hpp
 *
 * 06-07-2026 by madpl
 */
#pragma once

#include <array>
#include <cstdint>
#include <helicopter/Helicopter.hpp>
#include <input/InputSnapshot.hpp>
#include <network/NetworkPackets.hpp>


struct HelicopterSpawnPoint
{
	float x{0.0f};
	float y{0.0f};
	float z{0.0f};
	float yawDegrees{0.0f};
};


struct HelicopterSlot
{
	std::uint8_t slotIndex{0};
	SlotOccupancy occupancy{SlotOccupancy::Empty};
	
	bool locallyControlled{false};
	bool activeThisFrame{false};
	bool inputReceivedThisTick{false};
	
	std::uint32_t ownerPeerId{0};
	std::uint32_t lastProcessedTick{0};
	
	HelicopterSpawnPoint spawnPoint{};
	InputSnapshot lastInput{};
	Helicopter helicopter{};
};


namespace NetSpawn
{
	static constexpr std::array<HelicopterSpawnPoint, NetGame::MaxPlayers> DefaultSpawnPoints =
	{{
		{ 0.0f, 0.0f, 0.0f, 0.0f},
		{20.0f, 0.0f, 0.0f, 0.0f},
		{40.0f, 0.0f, 0.0f, 0.0f},
		{60.0f, 0.0f, 0.0f, 0.0f}
	}};
}
