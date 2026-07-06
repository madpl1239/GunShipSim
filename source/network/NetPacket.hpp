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
	float throttle = 0.0f;
	float yaw = 0.0f;
	float pitch = 0.0f;
	float roll = 0.0f;
	bool fire = false;
	bool reset = false;
};


struct StateSnapshotPacket
{
	std::uint32_t tick = 0;
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float yaw = 0.0f;
	float pitch = 0.0f;
	float roll = 0.0f;
};
