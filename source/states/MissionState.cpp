/*
 * MissionState.cpp
 * 
 * 05-07-2026 by madpl
 */
#include <states/MissionState.hpp>


MissionState::MissionState(StateManager& manager):
	IState(manager)
{
	// empty
}


void MissionState::onEnter()
{
	// tu przenosisz ładowanie terenu HGT, inicjalizację kamery, HUD
}


void MissionState::handleEvent(const sf::Event& event)
{
	// np. Escape -> m_manager.pushState(std::make_unique<PauseState>(m_manager));
}


void MissionState::update(float dt)
{
	m_helicopter.update(dt);
	m_camera.followCockpit(m_helicopter);
}


void MissionState::render()
{
	m_terrainRenderer.render(m_camera);
	m_helicopter.render(m_camera);
	m_hud.render();
}
