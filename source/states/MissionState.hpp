/*
 * MissionState.hpp
 * 
 * 05-072026 by madpl
 */
#pragma once

#include <core/IState.hpp>
#include <camera/Camera.hpp>
#include <helicopter/Helicopter.hpp>
#include <helicopter/HelicopterInput.hpp>
#include <hud/HUD.hpp>
#include <terrain/HGTLoader.hpp>
#include <terrain/TerrainData.hpp>
#include <terrain/TerrainRenderer.hpp>


class App;


class MissionState: public IState
{
public:
	MissionState(StateManager& manager, App& app);
	~MissionState() override;
	
	void onEnter() override;
	void onExit() override;
	void onEvent(const Event& event) override;
	void update(float dt) override;
	void render() override;
	
private:
	void updateCamera();
	void updateHud();
	void resetInputState();
	
	App& m_app;
	
	HUD m_hud;
	TerrainData m_terrain;
	TerrainRenderer m_renderer;
	Helicopter m_helicopter;
	Camera m_camera;
	HelicopterInputState m_inputState;
	
	float m_previousAltitude;
	float m_verticalSpeed;
	
	float m_camX;
	float m_camY;
	float m_camZ;
};
