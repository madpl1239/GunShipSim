/*
 * MissionState.hpp
 *
 * 05-07-2026 by madpl
 */
#pragma once

#include <array>
#include <cstdint>
#include <core/IState.hpp>
#include <core/InputEvents.hpp>
#include <core/InputSnapshot.hpp>
#include <network/NetworkConfig.hpp>
#include <hud/HUD.hpp>
#include <terrain/HGTLoader.hpp>
#include <terrain/TerrainData.hpp>
#include <terrain/TerrainRenderer.hpp>
#include <helicopter/Helicopter.hpp>
#include <camera/Camera.hpp>
#include <network/NetworkPackets.hpp>
#include <network/NetHost.hpp>
#include <network/NetClient.hpp>

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
	struct HelicopterSlot
	{
		bool occupied{false};
		std::uint32_t ownerPeerId{0};
		std::uint32_t lastProcessedTick{0};
		Helicopter helicopter;
		HelicopterInputState inputState{};
	};
	
	struct RenderState
	{
		float helicopterX{0.0f};
		float helicopterY{0.0f};
		float helicopterZ{0.0f};
		
		float helicopterYawDegrees{0.0f};
		float helicopterPitchDegrees{0.0f};
		float helicopterRollDegrees{0.0f};
		
		float camX{0.0f};
		float camY{0.0f};
		float camZ{0.0f};
		
		float targetX{0.0f};
		float targetY{0.0f};
		float targetZ{0.0f};
	};
	
	void initializeSlots();
	void initializeLocalOwnership();
	void placeSlotHelicopter(std::size_t slotIndex);
	
	void processHostNetworking();
	void processClientNetworking();
	
	void handlePendingJoinRequests();
	void handleIncomingPlayerInputs();
	
	bool findFreeSlotIndex(std::int32_t& outSlotIndex) const;
	std::uint32_t generatePeerId();
	
	void updateHostSimulation(float dt);
	void updateClientFromWorldState();
	
	void buildWorldStatePacket(WorldStatePacket& outPacket) const;
	void applyWorldStatePacket(const WorldStatePacket& packet);
	
	void writeSlotStateToPacket(std::size_t slotIndex, HelicopterSlotStatePacket& outPacket) const;
	void applyPacketToSlot(const HelicopterSlotStatePacket& packet);
	
	void updateHud();
	void captureCurrentRenderState();
	RenderState interpolateRenderState(float alpha) const;
	
	void resetInputState(HelicopterInputState& inputState);
	void applyLocalInputToState(HelicopterInputState& outState) const;
	void fillPlayerInputPacket(PlayerInputPacket& outPacket) const;
	
	std::int32_t getObservedSlotIndex() const;
	const Helicopter* getObservedHelicopter() const;
	
private:
	App& m_app;
	
	HUD m_hud;
	TerrainData m_terrain;
	TerrainRenderer m_renderer;
	Camera m_camera;
	InputSnapshot m_inputSnapshot;
	NetworkConfig m_networkConfig;
	
	std::array<HelicopterSlot, NetGame::MaxPlayers> m_slots;
	
	bool m_isHost{false};
	bool m_sessionReady{false};
	
	std::uint32_t m_localPeerId{0};
	std::int32_t m_localSlotIndex{NetGame::InvalidSlotIndex};
	
	std::uint32_t m_serverTick{0};
	std::uint32_t m_nextPeerId{1};
	
	NetHost m_host;
	NetClient m_client;
	
	WorldStatePacket m_latestWorldState{};
	bool m_hasLatestWorldState{false};
	
	float m_previousAltitude{0.0f};
	float m_verticalSpeed{0.0f};
	
	RenderState m_previousRenderState{};
	RenderState m_currentRenderState{};
};
