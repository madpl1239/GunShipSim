/*
 * MissionState.hpp
 *
 * 05-07-2026 by madpl
 */
#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <SFML/Graphics/RectangleShape.hpp>
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
#include <helicopter/HelicopterRenderer.hpp>

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
	
	void initializeUi();
	bool initializeTerrainAndRenderer();
	void initializeScene();
	void initializeGraphicsState();
	
	void handleWindowResizeEvent(const Event& event);
	void handleKeyPressedEvent(Event& event);
	
	void updateNetworking(float dt);
	void updateClientJoinAccept();
	void updateLocalSimulation(float dt);
	void updateHud();
	void updateNetworkDebugWindow(float dt);
	
	void renderHud();
	void renderDebugOverlay();
	
	void refreshDebugText();
	void captureDebugSnapshot();
	std::string buildDebugText() const;
	void appendHostDebugText(std::ostringstream& oss) const;
	void appendClientDebugText(std::ostringstream& oss) const;
	void appendLocalDebugText(std::ostringstream& oss) const;
	
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
	bool applyWorldStatePacket(const WorldStatePacket& packet);
	void resetNetworkDebugCounters();
	void renderNetworkHelicopters();
	void publishLocalHelicopterToOwnedSlot();
	
	App& m_app;
	
	HUD m_hud;
	
	TerrainData m_terrain;
	TerrainRenderer m_renderer;
	HelicopterRenderer m_helicopterRenderer;
	Helicopter m_helicopter;
	Camera m_camera;
	HelicopterInputState m_inputState;
	InputSnapshot m_inputSnapshot;
	NetworkConfig m_networkConfig;
	
	sf::Font m_statusFont;
	sf::Text m_statusText;
	sf::RectangleShape m_statusBackground;
	
	float m_previousAltitude;
	float m_verticalSpeed;
	
	RenderState m_previousRenderState;
	RenderState m_currentRenderState;
	
	bool m_joinRequestSent;
	std::uint32_t m_nextPeerId;
	
	std::array<HelicopterSlot, NetGame::MaxPlayers> m_slots;
	WorldStatePacket m_lastWorldState;
	bool m_hasWorldState;
	std::uint32_t m_lastAcceptedWorldStateTick;
	
	bool m_showDebugOverlay;
	bool m_freezeDebugOverlay;
	bool m_snapshotMode;
	float m_debugTextRefreshAccum;
	float m_debugTextRefreshInterval;
	std::string m_frozenDebugText;
	
	float m_debugNetAccumSeconds;
	
	std::uint32_t m_debugClientSentInputTotal;
	std::uint32_t m_debugClientSentInputPerSecond;
	std::uint32_t m_debugClientSentInputThisWindow;
	float m_debugClientLastSendDt;
	float m_debugClientMaxSendDtInWindow;
	
	std::uint32_t m_debugHostReceivedInputTotal;
	std::uint32_t m_debugHostReceivedInputPerSecond;
	std::uint32_t m_debugHostReceivedInputThisWindow;
	std::uint32_t m_debugHostLastInputPeerId;
	std::uint32_t m_debugHostLastInputTick;
	float m_debugHostLastReceiveDt;
	float m_debugHostMaxReceiveDtInWindow;
	
	std::uint32_t m_debugClientReceivedWorldTotal;
	std::uint32_t m_debugClientReceivedWorldPerSecond;
	std::uint32_t m_debugClientReceivedWorldThisWindow;
	std::uint32_t m_debugClientDroppedStaleWorldTotal;
	std::uint32_t m_debugClientDroppedStaleWorldThisWindow;
	std::uint32_t m_debugClientLastWorldStateTick;
	float m_debugClientLastWorldReceiveDt;
	float m_debugClientMaxWorldReceiveDtInWindow;
};
