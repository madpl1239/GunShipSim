/*
 * StateManager.hpp
 * 
 * 05-07-2026 by madpl
 */
#pragma once

#include <memory>
#include <vector>
#include <core/IState.hpp>


class StateManager
{
public:
	void pushState(std::unique_ptr<IState> state);
	void popState();
	void replaceState(std::unique_ptr<IState> state);
	
	bool isEmpty() const
	{
		return m_states.empty();
	}
	
	void handleEvent(const sf::Event& event);
	void update(float dt);
	void render();
	
private:
	void applyPendingChanges();
	
	std::vector<std::unique_ptr<IState>> m_states;
	std::vector<std::unique_ptr<IState>> m_pendingAdd;
	bool m_popRequested{false};
};
