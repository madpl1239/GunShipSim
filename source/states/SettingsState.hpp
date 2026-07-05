/*
 * SettingsState.hpp
 *
 * 05-07-2026 by madpl
 */
#pragma once

#include <core/IState.hpp>

#include <SFML/Graphics.hpp>

class App;

class SettingsState: public IState
{
public:
	SettingsState(StateManager& manager, App& app);
	~SettingsState() override = default;
	
	void onEnter() override;
	void onExit() override;
	void onEvent(const Event& event) override;
	void update(float dt) override;
	void render(float alpha) override;
	
private:
	App& m_app;
	sf::Font m_font;
	sf::Text m_title;
	sf::Text m_body;
	sf::Text m_hint;
};
