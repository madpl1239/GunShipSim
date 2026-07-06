/*
 * InputEvents.hpp
 *
 * 05-07-2026 by madpl
 */
#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <core/Event.hpp>


class QuitRequestedEvent: public Event
{
public:
	QuitRequestedEvent():
		Event(EventType::QuitRequested)
	{
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


class KeyEvent: public Event
{
public:
	KeyEvent(EventType type, sf::Keyboard::Key key):
		Event(type),
		m_key(key)
	{
	}
	
	sf::Keyboard::Key getKey() const
	{
		return m_key;
	}
	
private:
	sf::Keyboard::Key m_key;
};
