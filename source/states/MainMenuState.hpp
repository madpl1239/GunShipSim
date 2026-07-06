/*
 * MainMenuState.hpp
 *
 * 05-07-2026 by madpl
 */
#pragma once

#include <core/IState.hpp>

#include <SFML/Graphics.hpp>

class App;

class MainMenuState: public IState
{
public:
	MainMenuState(StateManager& manager, App& app);
	~MainMenuState() override = default;
	
	void onEnter() override;
	void onExit() override;
	void onEvent(Event& event) override;
	void update(float dt) override;
	void render(float alpha) override;
	
private:
	enum class Selection
	{
		Singleplayer = 0,
		Multiplayer,
		Settings,
		Exit
	};
	
	void activateSelection();
	void moveSelectionUp();
	void moveSelectionDown();
	void rebuildLayout();
	sf::Text makeText(const std::string& label, float y, bool highlighted) const;
	
	App& m_app;
	sf::Font m_font;
	sf::Text m_title;
	sf::Text m_hint;
	Selection m_selection;
	
	bool m_upWasDown;
	bool m_downWasDown;
	bool m_enterWasDown;
	bool m_escapeWasDown;
};
