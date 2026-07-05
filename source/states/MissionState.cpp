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
	m_camX(0.0f),
	m_camY(0.0f),
	m_camZ(0.0f)
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
	
	std::cout << "rawData.width = " << rawData.width << "\n";
	std::cout << "rawData.height = " << rawData.height << "\n";
	
	if(not m_terrain.buildFromHGT(rawData.samples, rawData.width, rawData.height,
									34.0f, 62.0f, 34.5f, 62.5f, 12000.0f, 256))
	{
		std::cerr << "Failed to build terrain data\n";
		
		return;
	}
	
	std::cout << "terrain width = " << m_terrain.getWorldSizeX() << "\n";
	std::cout << "terrain height = " << m_terrain.getWorldSizeZ() << "\n";
	std::cout << "spacing = " << m_terrain.getWorldSizeX() / float(m_terrain.getWidth() - 1)
				<< "\n";
	
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
	
	updateCamera();
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
		
		case EventType::FireCannonPressed:
		case EventType::FireCannonReleased:
		case EventType::LaunchMissilePressed:
		case EventType::LaunchMissileReleased:
		case EventType::KeyPressed:
		case EventType::KeyReleased:
		case EventType::MouseButtonPressed:
		case EventType::MouseButtonReleased:
		case EventType::MouseMoved:
		case EventType::CyclicRollChanged:
		case EventType::Unknown:
		default:
			break;
	}
}


void MissionState::update(float dt)
{
	m_helicopter.update(dt, m_terrain);
	
	if(dt > 0.0001f)
		m_verticalSpeed = (m_helicopter.getY() - m_previousAltitude) / dt;
	
	m_previousAltitude = m_helicopter.getY();
	
	updateCamera();
	updateHud();
}


void MissionState::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	m_renderer.render(m_camera.getViewProjectionMatrix(),
					  glm::vec3(m_camX, m_camY, m_camZ));
	
	m_app.getWindow().pushGLStates();
	m_hud.draw(m_app.getWindow());
	m_app.getWindow().popGLStates();
}


void MissionState::updateCamera()
{
	const float pi = 3.1415926535f;
	
	float yawRadians = m_helicopter.getYawDegrees() * pi / 180.0f;
	float pitchRadians = m_helicopter.getPitchDegrees() * pi / 180.0f;
	
	float forwardX = std::sin(yawRadians) * std::cos(pitchRadians);
	float forwardY = std::sin(pitchRadians);
	float forwardZ = -std::cos(yawRadians) * std::cos(pitchRadians);
	
	float cockpitForwardOffset = 1.2f;
	float cockpitUpOffset = 0.6f;
	float lookDistance = 250.0f;
	
	m_camX = m_helicopter.getX() + forwardX * cockpitForwardOffset;
	m_camY = m_helicopter.getY() + cockpitUpOffset;
	m_camZ = m_helicopter.getZ() + forwardZ * cockpitForwardOffset;
	
	float targetX = m_camX + forwardX * lookDistance;
	float targetY = m_camY + forwardY * lookDistance;
	float targetZ = m_camZ + forwardZ * lookDistance;
	
	m_camera.setPosition(m_camX, m_camY, m_camZ);
	m_camera.setTarget(targetX, targetY, targetZ);
	m_camera.setRollDegrees(-m_helicopter.getRollDegrees());
	m_camera.updateMatrices();
}


void MissionState::updateHud()
{
	m_hud.setHeadingDegrees(m_helicopter.getYawDegrees());
	m_hud.setAltitudeMeters(m_helicopter.getY());
	m_hud.setAltitudeAboveGroundMeters(m_helicopter.getAltitudeAboveGround());
	m_hud.setSpeedMetersPerSecond(m_helicopter.getSpeed());
	m_hud.setVerticalSpeedMetersPerSecond(m_verticalSpeed);
}


void MissionState::resetInputState()
{
	m_inputState.forwardInput = 0.0f;
	m_inputState.yawInput = 0.0f;
	m_inputState.verticalInput = 0.0f;
	m_inputState.brake = false;
}
