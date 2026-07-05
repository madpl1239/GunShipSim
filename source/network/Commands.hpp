/*
 * Commands.hpp
 * 
 * Net commands.
 * 
 * 05-07-2026 by madpl
 */
#pragma once

#include <cstdint>


enum class NetCommandType
{
	Invalid = 0,
	HelicopterControl,
	FireCannon,
	LaunchMissile,
	RespawnRequest
};


struct NetCommand
{
	explicit NetCommand(NetCommandType type):
		m_type(type)
	{
		// empty
	}
	
	virtual ~NetCommand() = default;
	
	NetCommandType getType() const
	{
		return m_type;
	}
	
private:
	NetCommandType m_type;
};


struct HelicopterControlCommand: public NetCommand
{
	HelicopterControlCommand():
		NetCommand(NetCommandType::HelicopterControl),
		playerId(0),
		tick(0),
		collective(0.0f),
		cyclicPitch(0.0f),
		cyclicRoll(0.0f),
		yawPedal(0.0f)
	{
		// empty
	}
	
	std::uint32_t playerId;
	std::uint32_t tick;
	float collective;
	float cyclicPitch;
	float cyclicRoll;
	float yawPedal;
};


struct FireCannonCommand: public NetCommand
{
public:
	FireCannonCommand():
		NetCommand(NetCommandType::FireCannon),
		playerId(0),
		tick(0),
		pressed(false)
	{
		// empty
	}
	
	std::uint32_t playerId;
	std::uint32_t tick;
	bool pressed;
};


struct LaunchMissileCommand: public NetCommand
{
public:
	LaunchMissileCommand():
		NetCommand(NetCommandType::LaunchMissile),
		playerId(0),
		tick(0)
	{
		// empty
	}
	
	std::uint32_t playerId;
	std::uint32_t tick;
};


struct RespawnRequestCommand: public NetCommand
{
public:
	RespawnRequestCommand():
		NetCommand(NetCommandType::RespawnRequest),
		playerId(0)
	{
		// empty
	}
	
	std::uint32_t playerId;
};
