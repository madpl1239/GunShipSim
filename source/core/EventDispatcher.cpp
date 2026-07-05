/*
 * EventDispatcher.cpp
 *
 * 05-07-2026 by madpl
 */
#include <algorithm>
#include <core/EventDispatcher.hpp>


void EventDispatcher::addListener(IListener* listener)
{
	if(listener == nullptr)
		return;
	
	const auto it = std::find(m_listeners.begin(), m_listeners.end(), listener);
	if(it != m_listeners.end())
		return;
	
	m_listeners.push_back(listener);
}


void EventDispatcher::removeListener(IListener* listener)
{
	const auto it = std::remove(m_listeners.begin(), m_listeners.end(), listener);
	m_listeners.erase(it, m_listeners.end());
}


void EventDispatcher::dispatch(const Event& event) const
{
	for(IListener* listener : m_listeners)
	{
		if(listener == nullptr)
			continue;
		
		listener->onEvent(event);
	}
}
