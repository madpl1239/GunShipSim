/*
 * NetPacket.hpp
 * 
 * 06-07-2026 by madpl
 */
#pragma once

#include <cstdint>


enum class NetPacketType : std::uint8_t
{
	InputSnapshot = 1,
	StateSnapshot = 2
};


struct InputSnapshotPacket
{
	std::uint32_t tick = 0;
	
	float forwardInput = 0.0f;
	float yawInput = 0.0f;
	float verticalInput = 0.0f;
	
	bool brake = false;
};


struct StateSnapshotPacket
{
	std::uint32_t tick = 0;
	
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	
	float yawDegrees = 0.0f;
	float pitchDegrees = 0.0f;
	float rollDegrees = 0.0f;
	
	float speed = 0.0f;
	float verticalSpeed = 0.0f;
	float altitudeAboveGround = 0.0f;
};
