/*
 * StateManager.cpp
 * 
 * 05-07-2026 by madpl
 */
#include <core/StateManager.hpp>


void StateManager::pushState(std::unique_ptr<IState> state)
{
	m_pendingAdd.push_back(std::move(state));
}


void StateManager::popState()
{
	m_popRequested = true;
}


void StateManager::replaceState(std::unique_ptr<IState> state)
{
	m_popRequested = true;
	
	m_pendingAdd.push_back(std::move(state));
}


void StateManager::applyPendingChanges()
{
	if(m_popRequested and !m_states.empty())
	{
		m_states.back()->onExit();
		m_states.pop_back();
		
		m_popRequested = false;
	}
	
	for(auto& s : m_pendingAdd)
	{
		m_states.push_back(std::move(s));
		m_states.back()->onEnter();
	}
	
	m_pendingAdd.clear();
}


void StateManager::handleEvent(const sf::Event& event)
{
	if(not m_states.empty())
		m_states.back()->handleEvent(event);
}


void StateManager::update(float dt)
{
	applyPendingChanges();
	
	if(not m_states.empty())
		m_states.back()->update(dt);
}


void StateManager::render()
{
	if(not m_states.empty())
		m_states.back()->render();
}
