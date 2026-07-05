/*
 * EventDispatcher.hpp
 * 
 * 05-07-2026 by madpl
 */
#pragma once

#include <functional>
#include <unordered_map>
#include <typeindex>
#include <any>
#include <vector>


class EventDispatcher
{
public:
	template<typename EventT>
	void subscribe(std::function<void(const EventT&)> callback)
	{
		m_listeners[typeid(EventT)].push_back([callback](const std::any& e)
		{
			callback(std::any_cast<const EventT&>(e));
			
		});
	}
	
	template<typename EventT>
	void publish(const EventT& event)
	{
		auto it = m_listeners.find(typeid(EventT));
		
		if(it == m_listeners.end())
			return;
		
		for(auto& fn : it->second)
			fn(event);
	}
	
private:
	std::unordered_map<std::type_index, std::vector<std::function<void(const std::any&)>>> m_listeners;
};
