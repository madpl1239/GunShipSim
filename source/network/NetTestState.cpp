/*
 * NetTestState.cpp
 * 
 * 06-07-2026 by madpl
 */
#include <SFML/Window/Keyboard.hpp>
#include <vector>
#include <iostream>
#include <sstream>
#include <terrain/HGTLoader.hpp>
#include <network/NetTestState.hpp>
#include <core/App.hpp>
#include <core/Event.hpp>
#include <core/EventType.hpp>
#include <core/InputEvents.hpp>
#include <network/NetCodec.hpp>


NetTestState::NetTestState(StateManager& manager, App& app):
	IState(manager),
	m_app(app),
	m_font(),
	m_infoText(),
	m_mode(Mode::Host),
	m_tick(0),
	m_host(),
	m_client(),
	m_terrain(),
	m_helicopter(),
	m_lastClientX(0.0f),
	m_lastClientY(0.0f),
	m_lastClientZ(0.0f),
	m_lastClientYaw(0.0f),
	m_lastClientPitch(0.0f),
	m_lastClientRoll(0.0f),
	m_lastClientSpeed(0.0f),
	m_lastClientVerticalSpeed(0.0f),
	m_lastClientAGL(0.0f)
{
	// empty
}


void NetTestState::onEnter()
{
	if(not m_font.loadFromFile("fonts/DejaVuSans.ttf"))
	{
		std::cerr << "Failed to load font\n";
		
		return;
	}
	
	m_infoText.setFont(m_font);
	m_infoText.setCharacterSize(20);
	m_infoText.setFillColor(sf::Color::White);
	m_infoText.setPosition(30.0f, 30.0f);
	
	HGTLoader loader;
	HGTLoader::Data rawData;
	
	if(not loader.load("res/terrain/N34E062.hgt", rawData))
	{
		std::cerr << "Failed to load HGT file for NetTestState\n";
		
		return;
	}
	
	if(not m_terrain.buildFromHGT(rawData.samples, rawData.width, rawData.height,
									34.0f, 62.0f, 34.5f, 62.5f, 12000.0f, 256))
	{
		std::cerr << "Failed to build terrain data for NetTestState\n";
		
		return;
	}
	
	m_helicopter.setPosition(0.0f, m_terrain.getHeightAtWorldPosition(0.0f, 0.0f) + 5.0f, 0.0f);
	m_helicopter.setYawDegrees(0.0f);
	
	m_mode = Mode::Host;
	m_tick = 0;
	
	m_lastClientX = 0.0f;
	m_lastClientY = 0.0f;
	m_lastClientZ = 0.0f;
	m_lastClientYaw = 0.0f;
	m_lastClientPitch = 0.0f;
	m_lastClientRoll = 0.0f;
	m_lastClientSpeed = 0.0f;
	m_lastClientVerticalSpeed = 0.0f;
	m_lastClientAGL = 0.0f;
	
	std::cout << "NetTest terrain loaded\n";
	std::cout << "terrain width = " << m_terrain.getWorldSizeX() << "\n";
	std::cout << "terrain height = " << m_terrain.getWorldSizeZ() << "\n";
	std::cout << "heli start y = " << m_helicopter.getY() << "\n";
	std::cout << "heli start agl = " << m_helicopter.getAltitudeAboveGround() << "\n";
	
	updateInfoText();
}


void NetTestState::onExit()
{
	m_host.stop();
	m_client.disconnect();
}


void NetTestState::onEvent(Event& event)
{
	switch(event.getType())
	{
		case EventType::QuitRequested:
		{
			m_app.stop();
			event.stopPropagation();
			
			break;
		}
		
		case EventType::KeyPressed:
		{
			KeyEvent& keyEvent = static_cast<KeyEvent&>(event);
			
			switch(keyEvent.getKey())
			{
				case sf::Keyboard::H:
				{
					m_host.stop();
					m_client.disconnect();
					
					if(m_host.start(55001))
						std::cout << "[HOST] started on port 55001\n";
					else
						std::cout << "[HOST] failed to start\n";
					
					m_mode = Mode::Host;
					updateInfoText();
					event.stopPropagation();
					
					break;
				}
				
				case sf::Keyboard::C:
				{
					m_host.stop();
					m_client.disconnect();
					
					if(m_client.connectTo(sf::IpAddress("127.0.0.1"), 55001))
						std::cout << "[CLIENT] connected to 127.0.0.1:55001\n";
					else
						std::cout << "[CLIENT] failed to connect\n";
					
					m_mode = Mode::Client;
					updateInfoText();
					event.stopPropagation();
					
					break;
				}
				
				case sf::Keyboard::Escape:
				{
					m_app.stop();
					event.stopPropagation();
					
					break;
				}
				
				case sf::Keyboard::Space:
				{
					if(m_mode == Mode::Client)
					{
						InputSnapshotPacket input{};
						input.tick = ++m_tick;
						input.forwardInput = 1.0f;
						input.yawInput = 0.25f;
						input.verticalInput = 0.0f;
						input.brake = false;
						
						if(m_client.sendInputSnapshot(input))
							std::cout << "[CLIENT] sent input tick=" << input.tick << "\n";
						else
							std::cout << "[CLIENT] send failed\n";
					}
					
					event.stopPropagation();
					
					break;
				}
				
				default:
					break;
			}
			
			break;
		}
		
		default:
			break;
	}
}


void NetTestState::update(float dt)
{
	if(m_mode == Mode::Host)
		updateHost(dt);
	else
		updateClient(dt);
}


void NetTestState::render(float alpha)
{
	auto& window = m_app.getWindow();
	
	window.pushGLStates();
	
	window.clear(sf::Color(15, 18, 24));
	window.draw(m_infoText);
	
	window.popGLStates();
}


void NetTestState::updateHost(float dt)
{
	InputSnapshotPacket input{};
	
	if(m_host.pollInputSnapshot(input))
	{
		HelicopterInputState inputState{};
		inputState.forwardInput = input.forwardInput;
		inputState.yawInput = input.yawInput;
		inputState.verticalInput = input.verticalInput;
		inputState.brake = input.brake;
		
		m_helicopter.setNetworkInputState(inputState);
		m_helicopter.update(dt, m_terrain);
		m_helicopter.clearNetworkInputOverride();
		
		StateSnapshotPacket state{};
		state.tick = input.tick;
		state.x = m_helicopter.getX();
		state.y = m_helicopter.getY();
		state.z = m_helicopter.getZ();
		state.yawDegrees = m_helicopter.getYawDegrees();
		state.pitchDegrees = m_helicopter.getPitchDegrees();
		state.rollDegrees = m_helicopter.getRollDegrees();
		state.speed = m_helicopter.getSpeed();
		state.verticalSpeed = m_helicopter.getVerticalSpeed();
		state.altitudeAboveGround = m_helicopter.getAltitudeAboveGround();
		
		if(m_host.sendStateSnapshot(state))
		{
			std::cout << "[HOST] tick=" << state.tick
						<< " pos=(" << state.x << ", " << state.y << ", " << state.z << ")"
						<< " yaw=" << state.yawDegrees
						<< " pitch=" << state.pitchDegrees
						<< " roll=" << state.rollDegrees
						<< " speed=" << state.speed
						<< " vs=" << state.verticalSpeed
						<< " agl=" << state.altitudeAboveGround << "\n";
		}
	}
}


void NetTestState::updateClient(float)
{
	StateSnapshotPacket state{};
	if(m_client.pollStateSnapshot(state))
	{
		m_lastClientX = state.x;
		m_lastClientY = state.y;
		m_lastClientZ = state.z;
		m_lastClientYaw = state.yawDegrees;
		m_lastClientPitch = state.pitchDegrees;
		m_lastClientRoll = state.rollDegrees;
		m_lastClientSpeed = state.speed;
		m_lastClientVerticalSpeed = state.verticalSpeed;
		m_lastClientAGL = state.altitudeAboveGround;
		
		std::cout << "[CLIENT] tick=" << state.tick
					<< " pos=(" << state.x << ", " << state.y << ", " << state.z << ")"
					<< " yaw=" << state.yawDegrees
					<< " pitch=" << state.pitchDegrees
					<< " roll=" << state.rollDegrees
					<< " speed=" << state.speed
					<< " vs=" << state.verticalSpeed
					<< " agl=" << state.altitudeAboveGround << "\n";
	}
}


void NetTestState::updateInfoText()
{
	std::ostringstream ss;
	
	ss << "NetTestState\n\n"
		<< "H - host on 127.0.0.1:55001\n"
		<< "C - client connect to 127.0.0.1:55001\n"
		<< "Space - send input from client\n"
		<< "Escape - quit\n\n"
		<< "Mode: " << (m_mode == Mode::Host ? "HOST" : "CLIENT") << "\n";
	
	if(m_mode == Mode::Client)
	{
		ss << "\nLast snapshot:\n"
			<< "pos=(" << m_lastClientX << ", " << m_lastClientY << ", " << m_lastClientZ << ")\n"
			<< "yaw=" << m_lastClientYaw
			<< " pitch=" << m_lastClientPitch
			<< " roll=" << m_lastClientRoll << "\n"
			<< "speed=" << m_lastClientSpeed
			<< " vs=" << m_lastClientVerticalSpeed
			<< " agl=" << m_lastClientAGL << "\n";
	}
	
	m_infoText.setString(ss.str());
}
