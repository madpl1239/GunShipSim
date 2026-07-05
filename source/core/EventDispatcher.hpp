/*
 * EventDispatcher.hpp
 * 
 * 05-07-2026 by madpl
 */
#pragma once

#include <vector>
#include <core/IListener.hpp>


class EventDispatcher
{
public:
	void addListener(IListener* listener);
	void removeListener(IListener* listener);
	void dispatch(const Event& event) const;
	
private:
	std::vector<IListener*> m_listeners;
};
