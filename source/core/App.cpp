/*
 * App.cpp
 *
 * 05-07-2026 by madpl
 */
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <GL/glew.h>
#include <iostream>
#include <memory>
#include <core/App.hpp>
#include <core/InputEvents.hpp>
#include <states/MissionState.hpp>


App::App():
	m_window(),
	m_stateManager(),
	m_dispatcher(),
	m_running(false),
	m_registeredStateListener(nullptr)
{
	// empty
}


bool App::initialize()
{
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 4;
	settings.majorVersion = 3;
	settings.minorVersion = 3;
	settings.attributeFlags = 0;
	
	m_window.create(sf::VideoMode(1280, 720), "GunSim v 0.1 by madpl 2026",
				 sf::Style::Titlebar | sf::Style::Close, settings);
	
	m_window.setVerticalSyncEnabled(false);
	m_window.setActive(true);
	
	glewExperimental = GL_TRUE;
	
	GLenum glewStatus = glewInit();
	if(glewStatus != GLEW_OK)
	{
		std::cerr << "glewInit failed\n";
		
		return false;
	}
	
	glGetError();
	
	glViewport(0, 0, 1280, 720);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor(0.60f, 0.75f, 0.95f, 1.0f);
	
	m_stateManager.pushState(std::make_unique<MissionState>(m_stateManager, *this));
	m_stateManager.update(0.0f);
	
	updateStateListener();
	
	m_running = true;
	
	return true;
}


void App::run()
{
	sf::Clock frameClock;
	float accumulator = 0.0f;
	
	while(m_running and m_window.isOpen())
	{
		float frameTime = frameClock.restart().asSeconds();
		if(frameTime > 0.25f)
			frameTime = 0.25f;
		
		accumulator += frameTime;
		
		processSystemEvents();
		
		while(accumulator >= FIXED_DT)
		{
			processRealtimeInput();
			
			m_stateManager.update(FIXED_DT);
			updateStateListener();
			
			accumulator -= FIXED_DT;
		}
		
		float alpha = accumulator / FIXED_DT;
		if(alpha < 0.0f)
			alpha = 0.0f;
		else if(alpha > 1.0f)
			alpha = 1.0f;
		
		m_stateManager.render(alpha);
		m_window.display();
	}
}


void App::stop()
{
	m_running = false;
	m_window.close();
}


void App::processSystemEvents()
{
	sf::Event sfEvent;
	
	while(m_window.pollEvent(sfEvent))
	{
		switch(sfEvent.type)
		{
			case sf::Event::Closed:
			{
				QuitRequestedEvent event;
				m_dispatcher.dispatch(event);
				break;
			}
			
			case sf::Event::Resized:
			{
				WindowResizedEvent event(sfEvent.size.width, sfEvent.size.height);
				m_dispatcher.dispatch(event);
				break;
			}
			
			case sf::Event::KeyPressed:
			{
				if(sfEvent.key.code == sf::Keyboard::Escape)
				{
					ActionEvent event(EventType::PauseRequested);
					m_dispatcher.dispatch(event);
				}
				
				break;
			}
			
			default:
				break;
		}
	}
}


void App::processRealtimeInput()
{
	float collective = 0.0f;
	float cyclicPitch = 0.0f;
	float cyclicRoll = 0.0f;
	float yawPedal = 0.0f;
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		cyclicPitch += 1.0f;
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		cyclicPitch -= 1.0f;
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		yawPedal -= 1.0f;
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		yawPedal += 1.0f;
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
		collective += 1.0f;
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::E))
		collective -= 1.0f;
	
	AxisChangedEvent collectiveEvent(EventType::CollectiveChanged, collective);
	m_dispatcher.dispatch(collectiveEvent);
	
	AxisChangedEvent cyclicPitchEvent(EventType::CyclicPitchChanged, cyclicPitch);
	m_dispatcher.dispatch(cyclicPitchEvent);
	
	AxisChangedEvent cyclicRollEvent(EventType::CyclicRollChanged, cyclicRoll);
	m_dispatcher.dispatch(cyclicRollEvent);
	
	AxisChangedEvent yawPedalEvent(EventType::YawPedalChanged, yawPedal);
	m_dispatcher.dispatch(yawPedalEvent);
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
	{
		ActionEvent event(EventType::FireCannonPressed);
		m_dispatcher.dispatch(event);
	}
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
	{
		ActionEvent event(EventType::LaunchMissilePressed);
		m_dispatcher.dispatch(event);
	}
}


void App::updateStateListener()
{
	IState* currentState = m_stateManager.getCurrentState();
	
	if(currentState == m_registeredStateListener)
		return;
	
	if(m_registeredStateListener != nullptr)
		m_dispatcher.removeListener(m_registeredStateListener);
	
	m_registeredStateListener = currentState;
	
	if(m_registeredStateListener != nullptr)
		m_dispatcher.addListener(m_registeredStateListener);
}
