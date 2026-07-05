/*
 * Event.hpp
 * 
 * 05-07-2026 by madpl
 */
#pragma once

#include <core/EventType.hpp>


class Event
{
public:
	explicit Event(EventType type):
		m_type(type)
	{
		// empty
	}
	
	virtual ~Event() = default;
	
	EventType getType() const
	{
		return m_type;
	}
	
private:
	EventType m_type;
};
