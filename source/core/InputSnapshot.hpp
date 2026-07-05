/*
 * InputSnapshot.hpp
 *
 * 05-07-2026 by madpl
 */
#pragma once

#include <cstdint>


struct InputSnapshot
{
	std::uint64_t tick;
	
	float collective;
	float cyclicPitch;
	float cyclicRoll;
	float yawPedal;
	
	bool fireCannon;
	bool launchMissile;
	bool pauseRequested;
};
