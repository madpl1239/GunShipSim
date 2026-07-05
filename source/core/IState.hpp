/*
 * IState.hpp
 * 
 * 04-07-2026 by madpl
 */
#pragma once

#include <SFML/Window/Event.hpp>

class StateManager;


class IState
{
public:
	explicit IState(StateManager& manager):
		m_manager(manager)
	{
		// empty
	}
	
	virtual ~IState() = default;
	
	virtual void onEnter(){}
	virtual void onExit(){}
	
	virtual void handleEvent(const sf::Event& event) = 0;
	virtual void update(float dt) = 0;
	virtual void render() = 0;
	
protected:
	StateManager& m_manager;
};
