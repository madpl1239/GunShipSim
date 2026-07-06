/*
 * EventRouter.cpp
 * 
 * 06-07-2026 by madpl
 */
#include <algorithm>
#include <core/EventRouter.hpp>
#include <core/Event.hpp>
#include <core/IEventListener.hpp>


void EventRouter::addListener(IEventListener* listener)
{
	if(listener == nullptr)
		return;
	
	auto it = std::find(m_listeners.begin(), m_listeners.end(), listener);
	if(it == m_listeners.end())
		m_listeners.push_back(listener);
}


void EventRouter::removeListener(IEventListener* listener)
{
	auto it = std::remove(m_listeners.begin(), m_listeners.end(), listener);
	m_listeners.erase(it, m_listeners.end());
}


void EventRouter::clear()
{
	m_listeners.clear();
}


void EventRouter::route(Event& event)
{
	for(IEventListener* listener: m_listeners)
	{
		if(listener == nullptr)
			continue;
		
		listener->onEvent(event);
		
		if(event.isHandled())
			break;
	}
}
