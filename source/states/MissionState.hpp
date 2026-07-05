/*
 * MissionState.hpp
 *
 * 05-07-2026 by madpl
 */
#pragma once

#include <core/IState.hpp>
#include <core/InputSnapshot.hpp>
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
	
	App& m_app;
	
	HUD m_hud;
	TerrainData m_terrain;
	TerrainRenderer m_renderer;
	Helicopter m_helicopter;
	Camera m_camera;
	HelicopterInputState m_inputState;
	InputSnapshot m_inputSnapshot;
	
	float m_previousAltitude;
	float m_verticalSpeed;
	
	RenderState m_previousRenderState;
	RenderState m_currentRenderState;
};
