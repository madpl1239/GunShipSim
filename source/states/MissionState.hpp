/*
 * MissionState.hpp
 *
 * 05-07-2026 by madpl
 */
#pragma once

#include <core/IState.hpp>
#include <core/InputSnapshot.hpp>
#include "hud/HUD.hpp"
#include "terrain/TerrainData.hpp"
#include "terrain/TerrainRenderer.hpp"
#include "terrain/HGTLoader.hpp"
#include "helicopter/Helicopter.hpp"
#include "camera/Camera.hpp"
#include "network/NetworkConfig.hpp"
#include "network/NetHost.hpp"
#include "network/NetClient.hpp"
#include "network/NetPacket.hpp"
#include <SFML/OpenGL.hpp>
#include <glm/vec3.hpp>

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
	void applyInputSnapshotToInputState();
	void applyInputStateToHelicopter();
	void updateLocal(float dt);
	void updateHost(float dt);
	void updateClient(float dt);
	void buildStateSnapshotPacket(StateSnapshotPacket& packet) const;
	void applyStateSnapshotPacket(const StateSnapshotPacket& packet);
	
	App& m_app;
	
	HUD m_hud;
	TerrainData m_terrain;
	TerrainRenderer m_renderer;
	Helicopter m_helicopter;
	Camera m_camera;
	
	HelicopterInputState m_inputState;
	InputSnapshot m_inputSnapshot;
	NetworkConfig m_networkConfig;
	
	NetHost m_netHost;
	NetClient m_netClient;
	
	float m_previousAltitude;
	float m_verticalSpeed;
	
	RenderState m_previousRenderState;
	RenderState m_currentRenderState;
};
