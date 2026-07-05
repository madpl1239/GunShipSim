/*
 * EventType.hpp
 * 
 * 05-07-2026 by mapdpl
 */
#pragma once


enum class EventType
{
	Unknown = 0,
	
	QuitRequested,
	WindowResized,
	
	KeyPressed,
	KeyReleased,
	
	MouseButtonPressed,
	MouseButtonReleased,
	MouseMoved,
	
	CollectiveChanged,
	CyclicPitchChanged,
	CyclicRollChanged,
	YawPedalChanged,
	
	FireCannonPressed,
	FireCannonReleased,
	
	LaunchMissilePressed,
	LaunchMissileReleased,
	
	PauseRequested
};
