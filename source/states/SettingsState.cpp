/*
 * SettingsState.cpp
 *
 * 05-07-2026 by madpl
 */
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <states/SettingsState.hpp>
#include <core/App.hpp>
#include <core/Event.hpp>
#include <core/EventType.hpp>


SettingsState::SettingsState(StateManager& manager, App& app):
	IState(manager),
	m_app(app),
	m_font(),
	m_title(),
	m_body(),
	m_hint()
{
	// empty
}


void SettingsState::onEnter()
{
	if(not m_font.loadFromFile("fonts/DejaVuSans.ttf"))
	{
		std::cerr << "Failed to load settings font\n";
		
		return;
	}
	
	m_title.setFont(m_font);
	m_title.setString("Settings");
	m_title.setCharacterSize(54);
	m_title.setFillColor(sf::Color::White);
	m_title.setPosition(80.0f, 60.0f);
	
	m_body.setFont(m_font);
	m_body.setString("Settings are a placeholder for now.\nAudio, graphics and controls will come later.");
	m_body.setCharacterSize(24);
	m_body.setFillColor(sf::Color(210, 210, 210));
	m_body.setPosition(80.0f, 170.0f);
	
	m_hint.setFont(m_font);
	m_hint.setString("Press Escape to go back.");
	m_hint.setCharacterSize(20);
	m_hint.setFillColor(sf::Color(180, 180, 180));
	m_hint.setPosition(80.0f, 310.0f);
}


void SettingsState::onExit()
{
}


void SettingsState::onEvent(const Event& event)
{
	switch(event.getType())
	{
		case EventType::QuitRequested:
			break;
			
		default:
			break;
	}
}


void SettingsState::update(float)
{
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
	{
		m_manager.popState();
	}
}


void SettingsState::render(float)
{
	auto& window = m_app.getWindow();
	window.clear(sf::Color(18, 22, 28));
	window.draw(m_title);
	window.draw(m_body);
	window.draw(m_hint);
}
