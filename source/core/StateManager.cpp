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


IState* StateManager::getCurrentState() const
{
	if(m_states.empty())
		return nullptr;
	
	return m_states.back().get();
}


void StateManager::applyPendingChanges()
{
	if(m_popRequested and !m_states.empty())
	{
		m_states.back()->onExit();
		m_states.pop_back();
		
		m_popRequested = false;
	}
	
	for(auto& state : m_pendingAdd)
	{
		m_states.push_back(std::move(state));
		m_states.back()->onEnter();
	}
	
	m_pendingAdd.clear();
}


void StateManager::update(float dt)
{
	applyPendingChanges();
	
	if(m_states.empty())
		return;
	
	m_states.back()->update(dt);
}


void StateManager::render()
{
	if(m_states.empty())
		return;
	
	m_states.back()->render();
}
