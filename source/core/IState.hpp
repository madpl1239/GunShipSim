/*
 * IState.hpp
 * 
 * 04-07-2026 by madpl
 */
#pragma once

#include <core/IEventListener.hpp>

class StateManager;


class IState: public IEventListener
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
	
	virtual void update(float dt) = 0;
	virtual void render(float alpha) = 0;
	
protected:
	StateManager& m_manager;
};
