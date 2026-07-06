/*
 * NetTestState.hpp
 * 
 * 06-07-2026 by madpl
 */
#pragma once

#include <cstdint>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <core/IState.hpp>
#include <network/NetHost.hpp>
#include <network/NetClient.hpp>
#include <helicopter/Helicopter.hpp>
#include <terrain/TerrainData.hpp>

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
	void updateInfoText();
	
	void applySnapshotToHelicopter(const StateSnapshotPacket& packet);
	
	App& m_app;
	
	sf::Font m_font;
	sf::Text m_infoText;
	
	Mode m_mode;
	std::uint32_t m_tick;
	
	NetHost m_host;
	NetClient m_client;
	
	TerrainData m_terrain;
	Helicopter m_helicopter;
	
	float m_lastClientX;
	float m_lastClientY;
	float m_lastClientZ;
	float m_lastClientYaw;
	float m_lastClientPitch;
	float m_lastClientRoll;
	float m_lastClientSpeed;
	float m_lastClientVerticalSpeed;
	float m_lastClientAGL;
};
