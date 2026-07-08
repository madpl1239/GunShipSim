/*
 * MissionState.hpp
 *
 * 05-07-2026 by madpl
 */
#pragma once

#include <array>
#include <cstdint>
#include <core/IState.hpp>
#include <core/InputSnapshot.hpp>
#include <camera/Camera.hpp>
#include <helicopter/Helicopter.hpp>
#include <helicopter/HelicopterInput.hpp>
#include <hud/HUD.hpp>
#include <terrain/HGTLoader.hpp>
#include <terrain/TerrainData.hpp>
#include <terrain/TerrainRenderer.hpp>
#include <network/HelicopterSlot.hpp>
#include <network/NetworkConfig.hpp>
#include <network/NetworkPackets.hpp>


class App;


class MissionState: public IState
{
public:
	MissionState(StateManager& manager, App& app);
	~MissionState() override;
	
	void onEnter() override;
	void onExit() override;
	void onEvent(Event& event) override;
	void update(float dt) override;
	void render(float alpha) override;
	
	void setInputSnapshot(const InputSnapshot& inputSnapshot);
	
private:
	struct RenderState
	{
		float helicopterX;
		float helicopterY;
		float helicopterZ;
		
		float helicopterYawDegrees;
		float helicopterPitchDegrees;
		float helicopterRollDegrees;
		
		float camX;
		float camY;
		float camZ;
		
		float targetX;
		float targetY;
		float targetZ;
	};
	
	void updateHud();
	void captureCurrentRenderState();
	RenderState interpolateRenderState(float alpha) const;
	void resetInputState();
	void applyInputSnapshot();
	
	void initializeNetworkSlots();
	void updateHostNetworking(float dt);
	void updateClientNetworking();
	void applyRemoteInputToSlot(HelicopterSlot& slot, const PlayerInputPacket& packet);
	void updateSlotHelicopter(HelicopterSlot& slot, float dt);
	void fillWorldStatePacket(WorldStatePacket& packet) const;
	void applyWorldStatePacket(const WorldStatePacket& packet);
	
	void updateNetworkDebugWindow(float dt);
	void resetNetworkDebugCounters();
	
	App& m_app;
	
	HUD m_hud;
	TerrainData m_terrain;
	TerrainRenderer m_renderer;
	Helicopter m_helicopter;
	Camera m_camera;
	HelicopterInputState m_inputState;
	InputSnapshot m_inputSnapshot;
	NetworkConfig m_networkConfig;
	
	sf::Font m_statusFont;
	sf::Text m_statusText;
	
	float m_previousAltitude;
	float m_verticalSpeed;
	
	RenderState m_previousRenderState;
	RenderState m_currentRenderState;
	
	bool m_joinRequestSent;
	std::uint32_t m_nextPeerId;
	
	std::array<HelicopterSlot, NetGame::MaxPlayers> m_slots;
	WorldStatePacket m_lastWorldState;
	bool m_hasWorldState;
	
	float m_debugNetAccumSeconds;
	
	std::uint32_t m_debugClientSentInputTotal;
	std::uint32_t m_debugClientSentInputPerSecond;
	std::uint32_t m_debugClientSentInputThisWindow;
	float m_debugClientLastSendDt;
	
	std::uint32_t m_debugHostReceivedInputTotal;
	std::uint32_t m_debugHostReceivedInputPerSecond;
	std::uint32_t m_debugHostReceivedInputThisWindow;
	std::uint32_t m_debugHostLastInputPeerId;
	std::uint32_t m_debugHostLastInputTick;
	float m_debugHostLastReceiveDt;
	
	std::uint32_t m_debugClientReceivedWorldTotal;
	std::uint32_t m_debugClientReceivedWorldPerSecond;
	std::uint32_t m_debugClientReceivedWorldThisWindow;
	std::uint32_t m_debugClientLastWorldStateTick;
	float m_debugClientLastWorldReceiveDt;
};
