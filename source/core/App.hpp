/*
 * App.hpp
 *
 * 05-07-2026 by madpl
 */
#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <core/EventDispatcher.hpp>
#include <core/StateManager.hpp>


class App
{
public:
	App();
	
	bool initialize();
	void run();
	void stop();
	
	sf::RenderWindow& getWindow()
	{
		return m_window;
	}
	
	StateManager& getStateManager()
	{
		return m_stateManager;
	}
	
	EventDispatcher& getEventDispatcher()
	{
		return m_dispatcher;
	}
	
private:
	void processSystemEvents();
	void processRealtimeInput();
	void updateStateListener();
	
	static constexpr float FIXED_DT = 1.0f / 60.0f;
	
	sf::RenderWindow m_window;
	StateManager m_stateManager;
	EventDispatcher m_dispatcher;
	bool m_running;
	IState* m_registeredStateListener;
};
