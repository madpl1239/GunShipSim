/*
 * IEventListener.hpp
 * 
 * 06-07-2026 by madpl
 */
#pragma once

class Event;


class IEventListener
{
public:
	virtual ~IEventListener() = default;
	virtual void onEvent(Event& event) = 0;
};
