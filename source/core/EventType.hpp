/*
 * EventType.hpp
 * 
 * 05-07-2026 by madpl
 */
#pragma once


enum class EventType
{
	Unknown = 0,
	
	QuitRequested,
	WindowResized,
	PauseRequested,
	
	StateChanged,
	
	MissionStarted,
	MissionEnded
};
