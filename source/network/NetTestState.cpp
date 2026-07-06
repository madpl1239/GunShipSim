#include "network/NetTestState.hpp"
#include "core/App.hpp"
#include "core/Event.hpp"
#include "core/EventType.hpp"
#include "core/InputEvents.hpp"
#include <SFML/Network/IpAddress.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <sstream>


NetTestState::NetTestState(StateManager& manager, App& app):
	IState(manager),
	m_app(app),
	m_font(),
	m_info(),
	m_mode(Mode::Host),
	m_host(),
	m_client(),
	m_tick(0),
	m_accumulator(0.0f)
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
	
	m_info.setFont(m_font);
	m_info.setCharacterSize(20);
	m_info.setFillColor(sf::Color::White);
	m_info.setPosition(30.0f, 30.0f);
	
	m_mode = Mode::Host;
	
	drawInfo();
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
					drawInfo();
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
					drawInfo();
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
						input.throttle = 1.0f;
						input.yaw = 0.25f;
						input.pitch = 0.0f;
						input.roll = 0.0f;
						input.fire = false;
						input.reset = false;
						
						if(m_client.sendInputSnapshot(input))
							std::cout << "[CLIENT] sent input tick=" << input.tick << "\n";
						else
							std::cout << "[CLIENT] send failed\n";
					}
					else
					{
						std::cout << "[HOST] press Space in CLIENT window\n";
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
	m_accumulator += dt;
	
	while(m_accumulator >= 0.05f)
	{
		if(m_mode == Mode::Host)
			updateHost(0.05f);
		else
			updateClient(0.05f);
		
		m_accumulator -= 0.05f;
	}
}


void NetTestState::render(float)
{
	auto& window = m_app.getWindow();
	window.clear(sf::Color(15, 18, 24));
	window.draw(m_info);
}


void NetTestState::updateHost(float)
{
	InputSnapshotPacket input{};
	
	if(m_host.pollInputSnapshot(input))
	{
		std::cout
		<< "[HOST] received input tick=" << input.tick
		<< " throttle=" << input.throttle
		<< " yaw=" << input.yaw
		<< "\n";
		
		StateSnapshotPacket state{};
		state.tick = input.tick;
		state.x = 100.0f + static_cast<float>(input.tick);
		state.y = 10.0f + input.throttle;
		state.z = 200.0f;
		state.yaw = input.yaw;
		state.pitch = input.pitch;
		state.roll = input.roll;
		
		if(m_host.sendStateSnapshot(state))
		{
			std::cout
			<< "[HOST] sent state tick=" << state.tick
			<< " x=" << state.x
			<< " y=" << state.y
			<< " z=" << state.z
			<< "\n";
		}
		
		else
		{
			std::cout << "[HOST] send state failed\n";
		}
	}
}

void NetTestState::updateClient(float)
{
	StateSnapshotPacket state{};
	
	if(m_client.pollStateSnapshot(state))
	{
		std::cout << "[CLIENT] received state tick=" << state.tick
					<< " x=" << state.x
					<< " y=" << state.y
					<< " z=" << state.z
					<< " yaw=" << state.yaw
					<< "\n";
	}
}


void NetTestState::drawInfo()
{
	std::ostringstream ss;
	
	ss << "NetTestState\n" << "\n"
		<< "H - start host on port 55001\n"
		<< "C - connect client to 127.0.0.1:55001\n"
		<< "Space - send packet from client\n"
		<< "Escape - quit\n"
		<< "\n"
		<< "Current mode: " << (m_mode == Mode::Host ? "HOST" : "CLIENT") << "\n";
	
	m_info.setString(ss.str());
}
