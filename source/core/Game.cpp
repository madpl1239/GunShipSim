/*
 * Game.cpp
 * 
 * 05-07-2026 by madpl
 */
#include <GL/glew.h>
#include <states/MainMenuState.hpp>
#include <core/Game.hpp>
#include <core/GameLoop.hpp>


Game::Game():
	m_window(sf::VideoMode(1280, 720), "GunShipSim v0.5 - madpl 2026",
			 sf::Style::Default, sf::ContextSettings(24, 8, 0, 4, 3))
{
	m_window.setActive(true);
	
	glewExperimental = GL_TRUE;
	glewInit();
	glEnable(GL_DEPTH_TEST);
	
	m_stateManager.pushState(std::make_unique<MainMenuState>(m_stateManager));
}


void Game::run()
{
	GameLoop loop(*this);
	
	loop.run();
}
