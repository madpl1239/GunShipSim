/*
 * MissionState.cpp
 *
 * 05-07-2026 by madpl
 */
#include <GL/glew.h>
#include <iostream>
#include <sstream>
#include <cmath>
#include <states/MissionState.hpp>
#include <core/App.hpp>
#include <core/Event.hpp>
#include <core/EventType.hpp>
#include <core/InputEvents.hpp>
#include <states/MainMenuState.hpp>
#include <network/NetworkPackets.hpp>


namespace
{
	float lerp(float a, float b, float alpha)
	{
		return a + (b - a) * alpha;
	}
	
	float normalizeAngleDegrees(float angleDegrees)
	{
		while(angleDegrees >= 360.0f)
			angleDegrees -= 360.0f;
		
		while(angleDegrees < 0.0f)
			angleDegrees += 360.0f;
		
		return angleDegrees;
	}
	
	float lerpAngleDegrees(float fromDegrees, float toDegrees, float alpha)
	{
		fromDegrees = normalizeAngleDegrees(fromDegrees);
		toDegrees = normalizeAngleDegrees(toDegrees);
		
		float delta = toDegrees - fromDegrees;
		
		if(delta > 180.0f)
			delta -= 360.0f;
		
		else if(delta < -180.0f)
			delta += 360.0f;
		
		return normalizeAngleDegrees(fromDegrees + delta * alpha);
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
	m_inputSnapshot{},
	m_networkConfig(app.getNetworkConfig()),
	m_previousAltitude(0.0f),
	m_verticalSpeed(0.0f),
	m_previousRenderState{},
	m_currentRenderState{},
	m_joinRequestSent(false),
	m_nextPeerId(1),
	m_slots{},
	m_lastWorldState{},
	m_hasWorldState(false)
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
	
	if(not m_statusFont.loadFromFile("fonts/DejaVuSans.ttf"))
	{
		std::cerr << "Failed to load status font\n";
		
		return;
	}
	
	if(m_networkConfig.mode == NetworkMode::Host)
	{
		if(not m_app.getNetHost().start(m_networkConfig.port))
			std::cerr << "Failed to start host on port " << m_networkConfig.port << "\n";
	}
	
	else if(m_networkConfig.mode == NetworkMode::Client)
	{
		if(m_app.getNetClient().connectTo(m_networkConfig.ipAddress, m_networkConfig.port))
		{
			if(m_app.getNetClient().sendJoinRequest())
				m_joinRequestSent = true;
			else
				std::cerr << "Failed to send join request\n";
		}
		
		else
			std::cerr << "Failed to connect client socket\n";
	}
	
	m_statusText.setFont(m_statusFont);
	m_statusText.setCharacterSize(18);
	m_statusText.setFillColor(sf::Color::White);
	m_statusText.setPosition(12.0f, 12.0f);
	
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
	
	initializeNetworkSlots();
	
	captureCurrentRenderState();
	m_previousRenderState = m_currentRenderState;
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearDepth(1.0f);
	glClearColor(0.60f, 0.75f, 0.95f, 1.0f);
	
	updateHud();
}


void MissionState::onExit()
{
	m_renderer.destroy();
	
	if(m_networkConfig.mode == NetworkMode::Host)
		m_app.getNetHost().stop();
	
	else if(m_networkConfig.mode == NetworkMode::Client)
		m_app.getNetClient().disconnect();
}


void MissionState::onEvent(Event& event)
{
	switch(event.getType())
	{
		case EventType::QuitRequested:
		{
			m_app.stop();
			event.stopPropagation();
			
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
			event.stopPropagation();
			
			break;
		}
		
		case EventType::KeyPressed:
		{
			KeyEvent& keyEvent = static_cast<KeyEvent&>(event);
			
			if(keyEvent.getKey() == sf::Keyboard::Escape)
			{
				// tu później pause menu albo powrót do main menu
				m_manager.replaceState(std::make_unique<MainMenuState>(m_manager, m_app));
				event.stopPropagation();
			}
			
			break;
		}
		
		default:
			break;
	}
}


void MissionState::update(float dt)
{
	if(m_networkConfig.mode == NetworkMode::Host)
		updateHostNetworking(dt);
	
	else if(m_networkConfig.mode == NetworkMode::Client)
	{
		JoinAcceptPacket acceptPacket{};
		if(m_app.getNetClient().pollJoinAccept(acceptPacket))
		{
			const std::int32_t slotIndex = m_app.getNetClient().getAssignedSlotIndex();
			if(slotIndex >= 0 && slotIndex < static_cast<std::int32_t>(m_slots.size()))
			{
				m_slots[slotIndex].occupancy = SlotOccupancy::Occupied;
				m_slots[slotIndex].locallyControlled = true;
				m_slots[slotIndex].ownerPeerId = m_app.getNetClient().getAssignedPeerId();
			}
		}
		
		updateClientNetworking();
	}
	
	m_previousRenderState = m_currentRenderState;
	
	applyInputSnapshot();
	
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
	m_app.getWindow().draw(m_statusText);
	m_app.getWindow().popGLStates();
}


void MissionState::setInputSnapshot(const InputSnapshot& inputSnapshot)
{
	m_inputSnapshot = inputSnapshot;
}


void MissionState::updateHud()
{
	m_hud.setHeadingDegrees(m_helicopter.getYawDegrees());
	m_hud.setAltitudeMeters(m_helicopter.getY());
	m_hud.setAltitudeAboveGroundMeters(m_helicopter.getAltitudeAboveGround());
	m_hud.setSpeedMetersPerSecond(m_helicopter.getSpeed());
	m_hud.setVerticalSpeedMetersPerSecond(m_verticalSpeed);
	
	if(m_networkConfig.mode == NetworkMode::Host)
	{
		const std::string& msg = m_app.getNetHost().getLastStatusMessage();
		m_statusText.setString(msg.empty() ? "Hosting session" : msg);
	}
	
	else if(m_networkConfig.mode == NetworkMode::Client)
	{
		const std::string& msg = m_app.getNetClient().getLastStatusMessage();
		m_statusText.setString(msg.empty() ? "Connecting to host..." : msg);
	}
	
	else
		m_statusText.setString("Local session");
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
	
	state.helicopterYawDegrees = lerpAngleDegrees(m_previousRenderState.helicopterYawDegrees,
												  m_currentRenderState.helicopterYawDegrees, alpha);
	
	state.helicopterPitchDegrees = lerpAngleDegrees(m_previousRenderState.helicopterPitchDegrees,
													m_currentRenderState.helicopterPitchDegrees, alpha);
	
	state.helicopterRollDegrees = lerpAngleDegrees(m_previousRenderState.helicopterRollDegrees,
												   m_currentRenderState.helicopterRollDegrees, alpha);
	
	state.camX = lerp(m_previousRenderState.camX, m_currentRenderState.camX, alpha);
	
	state.camY = lerp(m_previousRenderState.camY,
					  m_currentRenderState.camY, alpha);
	
	state.camZ = lerp(m_previousRenderState.camZ,
					  m_currentRenderState.camZ, alpha);
	
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


void MissionState::applyInputSnapshot()
{
	m_inputState.forwardInput = m_inputSnapshot.cyclicPitch;
	m_inputState.yawInput = m_inputSnapshot.yawPedal;
	m_inputState.verticalInput = m_inputSnapshot.collective;
	m_inputState.brake = (m_inputSnapshot.cyclicPitch < 0.0f);
}


void MissionState::initializeNetworkSlots()
{
	for(std::size_t i = 0; i < m_slots.size(); ++i)
	{
		HelicopterSlot& slot = m_slots[i];
		slot = HelicopterSlot{};
		slot.slotIndex = static_cast<std::uint8_t>(i);
		slot.spawnPoint = NetSpawn::DefaultSpawnPoints[i];
		
		slot.helicopter.setPosition(slot.spawnPoint.x,
									m_terrain.getHeightAtWorldPosition(slot.spawnPoint.x, slot.spawnPoint.z) + 5.0f,
									slot.spawnPoint.z);
		
		slot.helicopter.setYawDegrees(slot.spawnPoint.yawDegrees);
	}
	
	if(m_networkConfig.mode == NetworkMode::Host or m_networkConfig.mode == NetworkMode::Local)
	{
		m_slots[0].occupancy = SlotOccupancy::Occupied;
		m_slots[0].locallyControlled = true;
		m_slots[0].ownerPeerId = NetGame::HostPeerId;
	}
	
	if(m_networkConfig.mode == NetworkMode::Client)
	{
		m_slots[0].occupancy = SlotOccupancy::Occupied;
		m_slots[0].ownerPeerId = NetGame::HostPeerId;
	}
}


void MissionState::applyRemoteInputToSlot(HelicopterSlot& slot, const PlayerInputPacket& packet)
{
	slot.lastProcessedTick = packet.tick;
	slot.lastInput.tick = packet.tick;
	slot.lastInput.collective = packet.collective;
	slot.lastInput.cyclicPitch = packet.cyclicPitch;
	slot.lastInput.cyclicRoll = packet.cyclicRoll;
	slot.lastInput.yawPedal = packet.yawPedal;
	slot.lastInput.fireCannon = (packet.fireCannon != 0);
	slot.lastInput.launchMissile = (packet.launchMissile != 0);
	slot.lastInput.pauseRequested = (packet.pauseRequested != 0);
	slot.inputReceivedThisTick = true;
}


void MissionState::updateSlotHelicopter(HelicopterSlot& slot, float dt)
{
	HelicopterInputState inputState{};
	inputState.forwardInput = slot.lastInput.cyclicPitch;
	inputState.yawInput = slot.lastInput.yawPedal;
	inputState.verticalInput = slot.lastInput.collective;
	inputState.brake = (slot.lastInput.cyclicPitch < 0.0f);
	
	slot.helicopter.applyInput(inputState);
	slot.helicopter.update(dt, m_terrain);
}


void MissionState::fillWorldStatePacket(WorldStatePacket& packet) const
{
	packet = WorldStatePacket{};
	packet.serverTick = static_cast<std::uint32_t>(m_inputSnapshot.tick);
	packet.yourPeerId = NetGame::HostPeerId;
	packet.yourSlotIndex = 0;
	packet.activePlayerCount = 0;
	
	for(std::size_t i = 0; i < m_slots.size(); ++i)
	{
		const HelicopterSlot& slot = m_slots[i];
		HelicopterSlotStatePacket& out = packet.slots[i];
		
		out.slotIndex = static_cast<std::uint8_t>(i);
		out.occupied = (slot.occupancy == SlotOccupancy::Occupied) ? 1 : 0;
		out.ownerPeerId = slot.ownerPeerId;
		out.lastProcessedTick = slot.lastProcessedTick;
		
		out.x = slot.helicopter.getX();
		out.y = slot.helicopter.getY();
		out.z = slot.helicopter.getZ();
		
		out.yawDegrees = slot.helicopter.getYawDegrees();
		out.pitchDegrees = slot.helicopter.getPitchDegrees();
		out.rollDegrees = slot.helicopter.getRollDegrees();
		
		out.speedMetersPerSecond = slot.helicopter.getSpeed();
		out.verticalSpeedMetersPerSecond = 0.0f;
		out.altitudeAboveGroundMeters = slot.helicopter.getAltitudeAboveGround();
		
		if(out.occupied != 0)
			++packet.activePlayerCount;
	}
}


void MissionState::updateHostNetworking(float dt)
{
	sf::IpAddress joinAddress;
	std::uint16_t joinPort = 0;
	
	if(m_app.getNetHost().pollJoinRequest(joinAddress, joinPort))
	{
		const std::uint32_t peerId = m_nextPeerId++;
		const std::int32_t slotIndex = 1;
		
		if(m_app.getNetHost().registerPeer(joinAddress, joinPort, peerId, slotIndex))
		{
			m_slots[slotIndex].occupancy = SlotOccupancy::Occupied;
			m_slots[slotIndex].locallyControlled = false;
			m_slots[slotIndex].ownerPeerId = peerId;
			
			JoinAcceptPacket acceptPacket{};
			acceptPacket.accepted = 1;
			acceptPacket.assignedPeerId = peerId;
			acceptPacket.assignedSlotIndex = slotIndex;
			
			if(not m_app.getNetHost().sendJoinAccept(joinAddress, joinPort, acceptPacket))
				std::cerr << "Failed to send join accept\n";
		}
	}
	
	PlayerInputPacket inputPacket{};
	while(m_app.getNetHost().pollPlayerInput(inputPacket))
	{
		for(HelicopterSlot& slot : m_slots)
		{
			if(slot.occupancy == SlotOccupancy::Occupied &&
				slot.ownerPeerId == inputPacket.peerId)
			{
				applyRemoteInputToSlot(slot, inputPacket);
				updateSlotHelicopter(slot, dt);
				break;
			}
		}
	}
	
	m_slots[0].occupancy = SlotOccupancy::Occupied;
	m_slots[0].ownerPeerId = NetGame::HostPeerId;
	m_slots[0].helicopter = m_helicopter;
	m_slots[0].lastProcessedTick = static_cast<std::uint32_t>(m_inputSnapshot.tick);
	
	WorldStatePacket packet{};
	fillWorldStatePacket(packet);
	m_app.getNetHost().sendWorldStateToAll(packet);
}


void MissionState::applyWorldStatePacket(const WorldStatePacket& packet)
{
	m_lastWorldState = packet;
	m_hasWorldState = true;
	
	for(std::size_t i = 0; i < m_slots.size(); ++i)
	{
		const HelicopterSlotStatePacket& in = packet.slots[i];
		HelicopterSlot& slot = m_slots[i];
		
		slot.slotIndex = in.slotIndex;
		slot.occupancy = (in.occupied != 0) ? SlotOccupancy::Occupied : SlotOccupancy::Empty;
		slot.ownerPeerId = in.ownerPeerId;
		slot.lastProcessedTick = in.lastProcessedTick;
		
		if(slot.occupancy == SlotOccupancy::Empty)
			continue;
		
		slot.helicopter.setPosition(in.x, in.y, in.z);
		slot.helicopter.setYawDegrees(in.yawDegrees);
		slot.helicopter.setPitchDegrees(in.pitchDegrees);
		slot.helicopter.setRollDegrees(in.rollDegrees);
	}
}


void MissionState::updateClientNetworking()
{
	if(m_app.getNetClient().isAccepted())
	{
		PlayerInputPacket packet{};
		packet.peerId = m_app.getNetClient().getAssignedPeerId();
		packet.tick = static_cast<std::uint32_t>(m_inputSnapshot.tick);
		packet.collective = m_inputSnapshot.collective;
		packet.cyclicPitch = m_inputSnapshot.cyclicPitch;
		packet.cyclicRoll = m_inputSnapshot.cyclicRoll;
		packet.yawPedal = m_inputSnapshot.yawPedal;
		packet.brake = (m_inputSnapshot.cyclicPitch < 0.0f) ? 1 : 0;
		packet.fireCannon = m_inputSnapshot.fireCannon ? 1 : 0;
		packet.launchMissile = m_inputSnapshot.launchMissile ? 1 : 0;
		packet.pauseRequested = m_inputSnapshot.pauseRequested ? 1 : 0;
		
		m_app.getNetClient().sendPlayerInput(packet);
	}
	
	WorldStatePacket worldPacket{};
	while(m_app.getNetClient().pollWorldState(worldPacket))
		applyWorldStatePacket(worldPacket);
}
