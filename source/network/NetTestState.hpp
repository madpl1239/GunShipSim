#pragma once

#include <memory>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <core/IState.hpp>
#include <core/Event.hpp>
#include <network/NetHost.hpp>
#include <network/NetClient.hpp>

class App;
class StateManager;


class NetTestState : public IState
{
public:
	NetTestState(StateManager& manager, App& app);
	~NetTestState() override = default;
	
	void onEnter() override;
	void onExit() override;
	void onEvent(Event& event) override;
	void update(float dt) override;
	void render(float alpha) override;
	
private:
	enum class Mode
	{
		Host,
		Client
	};
	
	void updateHost(float dt);
	void updateClient(float dt);
	void drawInfo();
	
	App& m_app;
	sf::Font m_font;
	sf::Text m_info;
	Mode m_mode;
	NetHost m_host;
	NetClient m_client;
	std::uint32_t m_tick = 0;
	float m_accumulator = 0.0f;
};
