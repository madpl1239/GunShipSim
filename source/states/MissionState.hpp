/*
 * MissionState.hpp
 * 
 * 05-072026 by madpl
 */
#pragma once

#include <core/IState.hpp>
#include <helicopter/Helicopter.hpp>
#include <camera/Camera.hpp>
#include <terrain/TerrainRenderer.hpp>
#include <hud/HUD.hpp>


class MissionState : public IState
{
public:
	explicit MissionState(StateManager& manager);
	
	void onEnter() override;
	void handleEvent(const sf::Event& event) override;
	void update(float dt) override;
	void render() override;
	
private:
	Helicopter m_helicopter;
	Camera m_camera;
	TerrainRenderer m_terrainRenderer;
	HUD m_hud;
};
