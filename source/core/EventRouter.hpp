/*
 * EventRouter.hpp
 * 
 * 06-07-2026 by madpl
 */
#pragma once

#include <vector>

class Event;
class IEventListener;


class EventRouter
{
public:
	void addListener(IEventListener* listener);
	void removeListener(IEventListener* listener);
	void clear();
	
	void route(Event& event);
	
private:
	std::vector<IEventListener*> m_listeners;
};
