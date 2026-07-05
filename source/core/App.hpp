/*
 * App.hpp
 *
 * 05-07-2026 by madpl
 */
#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <cstdint>
#include <core/EventDispatcher.hpp>
#include <core/InputSnapshot.hpp>
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
	InputSnapshot sampleInputSnapshot() const;
	void updateStateListener();
	
	static constexpr float FIXED_DT = 1.0f / 60.0f;
	static constexpr int MAX_UPDATES_PER_FRAME = 8;
	
	sf::RenderWindow m_window;
	StateManager m_stateManager;
	EventDispatcher m_dispatcher;
	bool m_running;
	IState* m_registeredStateListener;
	bool m_pauseRequested;
	std::uint64_t m_tickCounter;
};
