/*
 * MissionState.cpp
 *
 * 05-07-2026 by madpl
 */
#include <GL/glew.h>
#include <iostream>
#include <cmath>
#include <states/MissionState.hpp>
#include <core/App.hpp>
#include <core/Event.hpp>
#include <core/EventType.hpp>
#include <core/InputEvents.hpp>


namespace
{
	float lerp(float a, float b, float alpha)
	{
		return a + (b - a) * alpha;
	}
}


MissionState::MissionState(StateManager& manager, App& app):
	IState(manager),
	m_app(app),
	m_hud(),
	m_terrain(),
	m_renderer(),
	m_helicopter(),
	m_camera(),
	m_inputState{},
	m_previousAltitude(0.0f),
	m_verticalSpeed(0.0f),
	m_previousRenderState{},
	m_currentRenderState{}
{
	resetInputState();
}


MissionState::~MissionState()
{
	m_renderer.destroy();
}


void MissionState::onEnter()
{
	if(not m_hud.initialize("fonts/DejaVuSans.ttf"))
	{
		std::cerr << "Failed to load HUD font\n";
		
		return;
	}
	
	HGTLoader loader;
	HGTLoader::Data rawData;
	
	if(not loader.load("res/terrain/N34E062.hgt", rawData))
	{
		std::cerr << "Failed to load HGT file\n";
		
		return;
	}
	
	if(not m_terrain.buildFromHGT(rawData.samples, rawData.width, rawData.height,
									34.0f, 62.0f, 34.5f, 62.5f, 12000.0f, 256))
	{
		std::cerr << "Failed to build terrain data\n";
		
		return;
	}
	
	if(not m_renderer.create(m_terrain))
	{
		std::cerr << "Failed to create terrain renderer\n";
		
		return;
	}
	
	m_helicopter.setPosition(0.0f, m_terrain.getHeightAtWorldPosition(0.0f, 0.0f) + 5.0f, 0.0f);
	m_helicopter.setYawDegrees(0.0f);
	
	m_camera.setPerspective(75.0f, 1280.0f / 720.0f, 0.3f, 100000.0f);
	m_camera.updateMatrices();
	
	m_previousAltitude = m_helicopter.getY();
	m_verticalSpeed = 0.0f;
	
	captureCurrentRenderState();
	m_previousRenderState = m_currentRenderState;
	
	updateHud();
}


void MissionState::onExit()
{
	m_renderer.destroy();
}


void MissionState::onEvent(const Event& event)
{
	switch(event.getType())
	{
		case EventType::QuitRequested:
		{
			m_app.stop();
			
			break;
		}
		
		case EventType::WindowResized:
		{
			const WindowResizedEvent& resizedEvent = static_cast<const WindowResizedEvent&>(event);
			
			glViewport(0, 0, static_cast<GLsizei>(resizedEvent.getWidth()),
					   static_cast<GLsizei>(resizedEvent.getHeight()));
			
			float aspect = static_cast<float>(resizedEvent.getWidth()) /
							static_cast<float>(resizedEvent.getHeight());
			
			m_camera.setPerspective(75.0f, aspect, 0.3f, 100000.0f);
			
			break;
		}
		
		case EventType::CollectiveChanged:
		{
			const AxisChangedEvent& axisEvent = static_cast<const AxisChangedEvent&>(event);
			
			m_inputState.verticalInput = axisEvent.getValue();
			
			break;
		}
		
		case EventType::CyclicPitchChanged:
		{
			const AxisChangedEvent& axisEvent = static_cast<const AxisChangedEvent&>(event);
			
			m_inputState.forwardInput = axisEvent.getValue();
			m_inputState.brake = (axisEvent.getValue() < 0.0f);
			
			break;
		}
		
		case EventType::YawPedalChanged:
		{
			const AxisChangedEvent& axisEvent = static_cast<const AxisChangedEvent&>(event);
			
			m_inputState.yawInput = axisEvent.getValue();
			
			break;
		}
		
		case EventType::PauseRequested:
		{
			m_app.stop();
			
			break;
		}
		
		default:
			break;
	}
}


void MissionState::update(float dt)
{
	m_previousRenderState = m_currentRenderState;
	
	m_helicopter.update(dt, m_terrain);
	
	if(dt > 0.0001f)
		m_verticalSpeed = (m_helicopter.getY() - m_previousAltitude) / dt;
	
	m_previousAltitude = m_helicopter.getY();
	
	captureCurrentRenderState();
	updateHud();
}


void MissionState::render(float alpha)
{
	const RenderState state = interpolateRenderState(alpha);
	
	m_camera.setPosition(state.camX, state.camY, state.camZ);
	m_camera.setTarget(state.targetX, state.targetY, state.targetZ);
	m_camera.setRollDegrees(-state.helicopterRollDegrees);
	m_camera.updateMatrices();
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	m_renderer.render(m_camera.getViewProjectionMatrix(),
					  glm::vec3(state.camX, state.camY, state.camZ));
	
	m_app.getWindow().pushGLStates();
	m_hud.draw(m_app.getWindow());
	m_app.getWindow().popGLStates();
}


void MissionState::updateHud()
{
	m_hud.setHeadingDegrees(m_helicopter.getYawDegrees());
	m_hud.setAltitudeMeters(m_helicopter.getY());
	m_hud.setAltitudeAboveGroundMeters(m_helicopter.getAltitudeAboveGround());
	m_hud.setSpeedMetersPerSecond(m_helicopter.getSpeed());
	m_hud.setVerticalSpeedMetersPerSecond(m_verticalSpeed);
}


void MissionState::captureCurrentRenderState()
{
	const float pi = 3.1415926535f;
	
	float helicopterX = m_helicopter.getX();
	float helicopterY = m_helicopter.getY();
	float helicopterZ = m_helicopter.getZ();
	
	float helicopterYawDegrees = m_helicopter.getYawDegrees();
	float helicopterPitchDegrees = m_helicopter.getPitchDegrees();
	float helicopterRollDegrees = m_helicopter.getRollDegrees();
	
	float yawRadians = helicopterYawDegrees * pi / 180.0f;
	float pitchRadians = helicopterPitchDegrees * pi / 180.0f;
	
	float forwardX = std::sin(yawRadians) * std::cos(pitchRadians);
	float forwardY = std::sin(pitchRadians);
	float forwardZ = -std::cos(yawRadians) * std::cos(pitchRadians);
	
	float cockpitForwardOffset = 1.2f;
	float cockpitUpOffset = 0.6f;
	float lookDistance = 250.0f;
	
	m_currentRenderState.helicopterX = helicopterX;
	m_currentRenderState.helicopterY = helicopterY;
	m_currentRenderState.helicopterZ = helicopterZ;
	
	m_currentRenderState.helicopterYawDegrees = helicopterYawDegrees;
	m_currentRenderState.helicopterPitchDegrees = helicopterPitchDegrees;
	m_currentRenderState.helicopterRollDegrees = helicopterRollDegrees;
	
	m_currentRenderState.camX = helicopterX + forwardX * cockpitForwardOffset;
	m_currentRenderState.camY = helicopterY + cockpitUpOffset;
	m_currentRenderState.camZ = helicopterZ + forwardZ * cockpitForwardOffset;
	
	m_currentRenderState.targetX = m_currentRenderState.camX + forwardX * lookDistance;
	m_currentRenderState.targetY = m_currentRenderState.camY + forwardY * lookDistance;
	m_currentRenderState.targetZ = m_currentRenderState.camZ + forwardZ * lookDistance;
}


MissionState::RenderState MissionState::interpolateRenderState(float alpha) const
{
	RenderState state{};
	
	state.helicopterX = lerp(m_previousRenderState.helicopterX,
							 m_currentRenderState.helicopterX, alpha);
	
	state.helicopterY = lerp(m_previousRenderState.helicopterY,
							 m_currentRenderState.helicopterY, alpha);
	
	state.helicopterZ = lerp(m_previousRenderState.helicopterZ,
							 m_currentRenderState.helicopterZ, alpha);
	
	state.helicopterYawDegrees = lerp(m_previousRenderState.helicopterYawDegrees,
									  m_currentRenderState.helicopterYawDegrees, alpha);
	
	state.helicopterPitchDegrees = lerp(m_previousRenderState.helicopterPitchDegrees,
										m_currentRenderState.helicopterPitchDegrees, alpha);
	
	state.helicopterRollDegrees = lerp(m_previousRenderState.helicopterRollDegrees,
									   m_currentRenderState.helicopterRollDegrees, alpha);
	
	state.camX = lerp(m_previousRenderState.camX, m_currentRenderState.camX, alpha);
	
	state.camY = lerp(m_previousRenderState.camY, m_currentRenderState.camY, alpha);
	
	state.camZ = lerp(m_previousRenderState.camZ, m_currentRenderState.camZ, alpha);
	
	state.targetX = lerp(m_previousRenderState.targetX,
						 m_currentRenderState.targetX, alpha);
	
	state.targetY = lerp(m_previousRenderState.targetY,
						 m_currentRenderState.targetY, alpha);
	
	state.targetZ = lerp(m_previousRenderState.targetZ,
						 m_currentRenderState.targetZ, alpha);
	
	return state;
}


void MissionState::resetInputState()
{
	m_inputState.forwardInput = 0.0f;
	m_inputState.yawInput = 0.0f;
	m_inputState.verticalInput = 0.0f;
	m_inputState.brake = false;
}
