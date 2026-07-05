/*
 * GameLoop.cpp
 * 
 * 05-07-2026 by madpl
 */
#include <SFML/System/Clock.hpp>
#include <algorithm>
#include <core/GameLoop.hpp>


void GameLoop::run()
{
	sf::Clock clock;
	float accumulator = 0.0f;
	
	while(m_game.window().isOpen() and !m_game.stateManager().isEmpty())
	{
		sf::Event event;
	
		while(m_game.window().pollEvent(event))
		{
			if(event.type == sf::Event::Closed)
				m_game.window().close();
			
			m_game.stateManager().handleEvent(event);
		}
		
		float frameTime = std::min(clock.restart().asSeconds(), 0.25f);
		accumulator += frameTime;
		
		while(accumulator >= FIXED_DT)
		{
			m_game.stateManager().update(FIXED_DT);
			
			accumulator -= FIXED_DT;
		}
		
		m_game.stateManager().render();
		m_game.window().display();
	}
}
