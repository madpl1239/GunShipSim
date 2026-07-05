/*
 * Game.hpp
 * 
 * 05-07-2026 by madpl
 */
#pragma once

#include <SFML/Window.hpp>
#include <core/StateManager.hpp>
#include <core/EventDispatcher.hpp>


class Game
{
public:
	Game();
	
	void run();
	
	sf::Window& window()
	{
		return m_window;
	}
	
	StateManager& stateManager()
	{
		return m_stateManager;
	}
	
	EventDispatcher& eventDispatcher()
	{
		return m_eventDispatcher;
	}
	
private:
	sf::Window m_window;
	StateManager m_stateManager;
	EventDispatcher m_eventDispatcher;
};
