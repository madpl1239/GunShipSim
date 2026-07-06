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
		m_type(type),
		m_handled(false)
	{
		// empty
	}
	
	virtual ~Event() = default;
	
	EventType getType() const
	{
		return m_type;
	}
	
	bool isHandled() const
	{
		return m_handled;
	}
	
	void stopPropagation()
	{
		m_handled = true;
	}
	
protected:
	EventType m_type;
	bool m_handled;
};
