/*
 * MainMenuState.cpp
 *
 * 05-07-2026 by madpl
 */
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <iostream>
#include <memory>
#include <states/MainMenuState.hpp>
#include <core/App.hpp>
#include <core/Event.hpp>
#include <core/EventType.hpp>
#include <core/InputEvents.hpp>
#include <states/MissionState.hpp>
#include <states/SettingsState.hpp>


MainMenuState::MainMenuState(StateManager& manager, App& app):
	IState(manager),
	m_app(app),
	m_font(),
	m_title(),
	m_hint(),
	m_selection(Selection::Singleplayer),
	m_upWasDown(false),
	m_downWasDown(false),
	m_enterWasDown(false),
	m_escapeWasDown(false)
{
	// empty
}


void MainMenuState::onEnter()
{
	if(not m_font.loadFromFile("fonts/DejaVuSans.ttf"))
	{
		std::cerr << "Failed to load menu font\n";
		
		return;
	}
	
	m_title.setFont(m_font);
	m_title.setString("GunShip Simulator");
	m_title.setCharacterSize(56);
	m_title.setFillColor(sf::Color::White);
	m_title.setPosition(80.0f, 60.0f);
	
	m_hint.setFont(m_font);
	m_hint.setCharacterSize(20);
	m_hint.setFillColor(sf::Color(220, 220, 220));
	m_hint.setString("Up/Down: choose   Enter: select   Escape: exit");
	
	rebuildLayout();
}


void MainMenuState::onExit()
{
}


void MainMenuState::onEvent(Event& event)
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
					
				case sf::Keyboard::Escape:
					m_app.stop();
					event.stopPropagation();
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


void MainMenuState::update(float)
{
	// no-op
}


void MainMenuState::render(float)
{
	auto& window = m_app.getWindow();
	
	window.pushGLStates();
	
	window.clear(sf::Color(18, 22, 28));
	window.draw(m_title);
	window.draw(makeText("Singleplayer", 220.0f, m_selection == Selection::Singleplayer));
	window.draw(makeText("Settings", 290.0f, m_selection == Selection::Settings));
	window.draw(makeText("Exit", 360.0f, m_selection == Selection::Exit));
	window.draw(m_hint);
	
	window.popGLStates();
}


void MainMenuState::activateSelection()
{
	switch(m_selection)
	{
		case Selection::Singleplayer:
			m_manager.replaceState(std::make_unique<MissionState>(m_manager, m_app));
			break;
			
		case Selection::Settings:
			m_manager.pushState(std::make_unique<SettingsState>(m_manager, m_app));
			break;
			
		case Selection::Exit:
			m_app.stop();
			break;
	}
}


void MainMenuState::moveSelectionUp()
{
	if(m_selection == Selection::Singleplayer)
		m_selection = Selection::Exit;
	else
		m_selection = static_cast<Selection>(static_cast<int>(m_selection) - 1);
}


void MainMenuState::moveSelectionDown()
{
	if(m_selection == Selection::Exit)
		m_selection = Selection::Singleplayer;
	else
		m_selection = static_cast<Selection>(static_cast<int>(m_selection) + 1);
}


void MainMenuState::rebuildLayout()
{
	m_hint.setPosition(80.0f, 470.0f);
}


sf::Text MainMenuState::makeText(const std::string& label, float y, bool highlighted) const
{
	sf::Text text;
	text.setFont(m_font);
	text.setString(label);
	text.setCharacterSize(highlighted ? 34 : 28);
	text.setFillColor(highlighted ? sf::Color(255, 244, 200) : sf::Color(190, 190, 190));
	text.setPosition(100.0f, y);
	
	return text;
}
