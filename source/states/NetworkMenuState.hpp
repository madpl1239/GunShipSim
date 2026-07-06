/*
 * NetworkMenuState.hpp
 *
 * 06-07-2026 by madpl
 */
#pragma once

#include <SFML/Graphics.hpp>
#include <core/IState.hpp>
#include <network/NetworkConfig.hpp>

class App;


class NetworkMenuState : public IState
{
public:
	NetworkMenuState(StateManager& manager, App& app);
	~NetworkMenuState() override = default;
	
	void onEnter() override;
	void onExit() override;
	void onEvent(Event& event) override;
	void update(float dt) override;
	void render(float alpha) override;
	
private:
	enum class Selection
	{
		Local = 0,
		Host,
		Client,
		Ip,
		Port,
		Start,
		Back
	};
	
	void rebuildLayout();
	void moveSelectionUp();
	void moveSelectionDown();
	void activateSelection();
	void editIpChar(sf::Keyboard::Key key);
	void applySelectionToConfig();
	void syncText();
	
	App& m_app;
	sf::Font m_font;
	sf::Text m_title;
	sf::Text m_hint;
	sf::Text m_lines[7];
	
	Selection m_selection;
	NetworkConfig m_config;
	std::string m_ipBuffer;
	bool m_cursorVisible;
	float m_cursorBlink;
};
