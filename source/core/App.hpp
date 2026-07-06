/*
 * App.hpp
 *
 * 05-07-2026 by madpl
 */
#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <cstdint>
#include <core/EventRouter.hpp>
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
	
private:
	void processSystemEvents();
	InputSnapshot sampleInputSnapshot() const;
	void updateStateListener();
	
	static constexpr float FIXED_DT = 1.0f / 60.0f;
	static constexpr int MAX_UPDATES_PER_FRAME = 8;
	
	sf::RenderWindow m_window;
	StateManager m_stateManager;
	EventRouter m_eventRouter;
	bool m_running;
	IState* m_registeredStateListener;
	std::uint64_t m_tickCounter;
};
