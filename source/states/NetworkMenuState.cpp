/*
 * NetworkMenuState.cpp
 *
 * 06-07-2026 by madpl
 */
#include <sstream>
#include <iostream>
#include <memory>
#include <states/NetworkMenuState.hpp>
#include <core/App.hpp>
#include <core/Event.hpp>
#include <core/EventType.hpp>
#include <core/InputEvents.hpp>
#include <states/MainMenuState.hpp>
#include <states/MissionState.hpp>
#include <core/Defines.hpp>


namespace
{
	constexpr int kMaxIpLength = 15;
}


NetworkMenuState::NetworkMenuState(StateManager& manager, App& app):
	IState(manager),
	m_app(app),
	m_font(),
	m_title(),
	m_hint(),
	m_lines{},
	m_selection(Selection::Local),
	m_config(app.getNetworkConfig()),
	m_ipBuffer(m_config.ipAddress),
	m_cursorVisible(true),
	m_cursorBlink(0.0f)
{
	// empty
}


void NetworkMenuState::onEnter()
{
	if(not m_font.loadFromFile("fonts/DejaVuSans.ttf"))
	{
		std::cerr << "Failed to load network menu font\n";
		
		return;
	}
	
	m_title.setFont(m_font);
	m_title.setString("Network Setup");
	m_title.setCharacterSize(48);
	m_title.setFillColor(sf::Color::White);
	m_title.setPosition(40.0f, 30.0f);
	
	m_hint.setFont(m_font);
	m_hint.setCharacterSize(18);
	m_hint.setFillColor(sf::Color(220, 220, 220));
	m_hint.setPosition(HINT_POSX, HINT_POSY);
	m_hint.setString("Up/Down: choose  Left/Right: change  Enter: activate  Esc: back");
	
	for(auto& line : m_lines)
	{
		line.setFont(m_font);
		line.setCharacterSize(26);
		line.setFillColor(sf::Color(200, 200, 200));
	}
	
	syncText();
	rebuildLayout();
}


void NetworkMenuState::onExit()
{
	m_app.setNetworkConfig(m_config);
}


void NetworkMenuState::onEvent(Event& event)
{
	switch(event.getType())
	{
		case EventType::QuitRequested:
		{
			m_app.stop();
			event.stopPropagation();
			
			break;
		}
		
		case EventType::KeyPressed:
		{
			KeyEvent& keyEvent = static_cast<KeyEvent&>(event);
			
			if(keyEvent.getKey() == sf::Keyboard::Escape)
			{
				m_app.setNetworkConfig(m_config);
				m_manager.replaceState(std::make_unique<MainMenuState>(m_manager, m_app));
				event.stopPropagation();
				
				break;
			}
			
			switch(keyEvent.getKey())
			{
				case sf::Keyboard::Up:
					moveSelectionUp();
					event.stopPropagation();
					break;
					
				case sf::Keyboard::Down:
					moveSelectionDown();
					event.stopPropagation();
					break;
					
				case sf::Keyboard::Enter:
				case sf::Keyboard::Space:
					activateSelection();
					event.stopPropagation();
					break;
					
				case sf::Keyboard::Left:
				case sf::Keyboard::Right:
					if(m_selection == Selection::Local or
						m_selection == Selection::Host or
						m_selection == Selection::Client)
					{
						activateSelection();
						syncText();
						rebuildLayout();
						event.stopPropagation();
					}
					
					break;
					
				default:
					break;
			}
			
			break;
		}
		
		default:
			break;
	}
}


void NetworkMenuState::update(float dt)
{
	m_cursorBlink += dt;
	
	if(m_cursorBlink >= 0.5f)
	{
		m_cursorBlink = 0.0f;
		m_cursorVisible = !m_cursorVisible;
		
		syncText();
	}
}


void NetworkMenuState::render(float)
{
	auto& window = m_app.getWindow();
	
	window.pushGLStates();
	
	window.clear(BACKGROUND_COLOR2);
	window.draw(m_title);
	
	for(auto& line : m_lines)
		window.draw(line);
	
	window.draw(m_hint);
	
	window.popGLStates();
}


void NetworkMenuState::rebuildLayout()
{
	for(int i = 0; i < 7; ++i)
	{
		m_lines[i].setPosition(60.0f, 100.0f + float(i) * 28.0f);
		m_lines[i].setFillColor(i == static_cast<int>(m_selection) ? HINT_COLOR1 : HINT_COLOR2);
	}
}


void NetworkMenuState::moveSelectionUp()
{
	int current = static_cast<int>(m_selection);
	
	current = (current == 0) ? 6 : current - 1;
	m_selection = static_cast<Selection>(current);
	
	rebuildLayout();
	syncText();
}


void NetworkMenuState::moveSelectionDown()
{
	int current = static_cast<int>(m_selection);
	
	current = (current == 6) ? 0 : current + 1;
	m_selection = static_cast<Selection>(current);
	
	rebuildLayout();
	syncText();
}


void NetworkMenuState::activateSelection()
{
	switch(m_selection)
	{
		case Selection::Local:
			m_config.mode = NetworkMode::Local;
			
			break;
		
		case Selection::Host:
			m_config.mode = NetworkMode::Host;
			
			break;
		
		case Selection::Client:
			m_config.mode = NetworkMode::Client;
			
			break;
		
		case Selection::Ip:
			if(m_ipBuffer.size() < kMaxIpLength)
				m_ipBuffer += "1";
			
			m_config.ipAddress = m_ipBuffer;
			
			break;
		
		case Selection::Port:
			++m_config.port;
			
			if(m_config.port > 65000)
				m_config.port = 55001;
			
			
			break;
		
		case Selection::Start:
			m_app.setNetworkConfig(m_config);
			m_manager.replaceState(std::make_unique<MissionState>(m_manager, m_app));
			
			break;
		
		case Selection::Back:
			m_app.setNetworkConfig(m_config);
			m_manager.replaceState(std::make_unique<MainMenuState>(m_manager, m_app));
			
			break;
	}
	
	syncText();
	rebuildLayout();
}


void NetworkMenuState::editIpChar(sf::Keyboard::Key)
{
}


void NetworkMenuState::applySelectionToConfig()
{
}


void NetworkMenuState::syncText()
{
	std::ostringstream ss0, ss1, ss2, ss3, ss4, ss5, ss6;
	
	ss0 << "Mode: LOCAL";
	ss1 << "Mode: HOST";
	ss2 << "Mode: CLIENT";
	ss3 << "IP: " << m_config.ipAddress;
	ss4 << "Port: " << m_config.port;
	ss5 << "Start mission";
	ss6 << "Back";
	
	m_lines[0].setString(ss0.str());
	m_lines[1].setString(ss1.str());
	m_lines[2].setString(ss2.str());
	m_lines[3].setString(ss3.str());
	m_lines[4].setString(ss4.str());
	m_lines[5].setString(ss5.str());
	m_lines[6].setString(ss6.str());
	
	if(m_cursorVisible and m_selection == Selection::Ip)
		m_lines[3].setString("IP: " + m_config.ipAddress + "_");
}
