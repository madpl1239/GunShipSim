/*
 * InputEvents.hpp
 * 
 * 05-07-2026 by madpl
 */
#pragma once

#include <core/Event.hpp>


class QuitRequestedEvent: public Event
{
public:
	QuitRequestedEvent():
		Event(EventType::QuitRequested)
	{
		// empty
	}
};


class WindowResizedEvent: public Event
{
public:
	WindowResizedEvent(unsigned width, unsigned height):
		Event(EventType::WindowResized),
		m_width(width),
		m_height(height)
	{
		// empty
	}
	
	unsigned getWidth() const
	{
		return m_width;
	}
	
	unsigned getHeight() const
	{
		return m_height;
	}
	
private:
	unsigned m_width;
	unsigned m_height;
};


class AxisChangedEvent: public Event
{
public:
	AxisChangedEvent(EventType type, float value):
		Event(type),
		m_value(value)
	{
		// empty
	}
	
	float getValue() const
	{
		return m_value;
	}
	
private:
	float m_value;
};


class ActionEvent: public Event
{
public:
	explicit ActionEvent(EventType type):
		Event(type)
	{
		// empty
	}
};
