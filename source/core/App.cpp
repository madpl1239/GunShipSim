/*
 * App.cpp
 *
 * 05-07-2026 by madpl
 */
#include <GL/glew.h>
#include <iostream>
#include <memory>
#include <SFML/Window/Event.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <core/App.hpp>
#include <core/EventRouter.hpp>
#include <core/InputEvents.hpp>
#include <core/IState.hpp>
#include <states/MainMenuState.hpp>
#include <states/MissionState.hpp>
#include <network/NetTestState.hpp>


App::App():
	m_window(),
	m_stateManager(),
	m_eventRouter(),
	m_running(false),
	m_registeredStateListener(nullptr),
	m_tickCounter(0)
{
	// empty
}


bool App::initialize()
{
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 2;
	settings.majorVersion = 3;
	settings.minorVersion = 3;
	settings.attributeFlags = 0;
	
	// 1280, 720
	m_window.create(sf::VideoMode(800, 600), "GunSim v 0.1 by madpl 2026",
				 sf::Style::Titlebar | sf::Style::Close, settings);
	
	m_window.setKeyRepeatEnabled(false);
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
	glClearDepth(1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor(0.60f, 0.75f, 0.95f, 1.0f);
	
	// m_stateManager.pushState(std::make_unique<NetTestState>(m_stateManager, *this));
	m_stateManager.pushState(std::make_unique<MainMenuState>(m_stateManager, *this));
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
		
		int updatesThisFrame = 0;
		
		while(accumulator >= FIXED_DT and updatesThisFrame < MAX_UPDATES_PER_FRAME)
		{
			InputSnapshot inputSnapshot = sampleInputSnapshot();
			
			IState* currentState = m_stateManager.getCurrentState();
			MissionState* missionState = dynamic_cast<MissionState*>(currentState);
			if(missionState != nullptr)
				missionState->setInputSnapshot(inputSnapshot);
			
			m_stateManager.update(FIXED_DT);
			updateStateListener();
			
			accumulator -= FIXED_DT;
			++updatesThisFrame;
			++m_tickCounter;
		}
		
		if(updatesThisFrame == MAX_UPDATES_PER_FRAME and accumulator >= FIXED_DT)
			accumulator = 0.0f;
		
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
				m_eventRouter.route(event);
				
				break;
			}
			
			case sf::Event::Resized:
			{
				WindowResizedEvent event(sfEvent.size.width, sfEvent.size.height);
				m_eventRouter.route(event);
				
				break;
			}
			
			case sf::Event::KeyPressed:
			{
				KeyEvent event(EventType::KeyPressed, sfEvent.key.code);
				m_eventRouter.route(event);
				
				break;
			}
			
			default:
				break;
		}
	}
}


InputSnapshot App::sampleInputSnapshot() const
{
	InputSnapshot snapshot{};
	snapshot.tick = m_tickCounter;
	snapshot.collective = 0.0f;
	snapshot.cyclicPitch = 0.0f;
	snapshot.cyclicRoll = 0.0f;
	snapshot.yawPedal = 0.0f;
	snapshot.fireCannon = false;
	snapshot.launchMissile = false;
	snapshot.pauseRequested = false;
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		snapshot.cyclicPitch += 1.0f;
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		snapshot.cyclicPitch -= 1.0f;
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		snapshot.yawPedal -= 1.0f;
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		snapshot.yawPedal += 1.0f;
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
		snapshot.collective += 1.0f;
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::E))
		snapshot.collective -= 1.0f;
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		snapshot.fireCannon = true;
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
		snapshot.launchMissile = true;
	
	return snapshot;
}


void App::updateStateListener()
{
	IState* currentState = m_stateManager.getCurrentState();
	
	if(currentState == m_registeredStateListener)
		return;
	
	if(m_registeredStateListener != nullptr)
		m_eventRouter.removeListener(m_registeredStateListener);
	
	m_registeredStateListener = currentState;
	
	if(m_registeredStateListener != nullptr)
		m_eventRouter.addListener(m_registeredStateListener);
}


NetworkConfig& App::getNetworkConfig()
{
	return m_networkConfig;
}


const NetworkConfig& App::getNetworkConfig() const
{
	return m_networkConfig;
}


void App::setNetworkConfig(const NetworkConfig& config)
{
	m_networkConfig = config;
}
