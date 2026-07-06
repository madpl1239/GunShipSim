/*
 * MissionState.cpp
 *
 * 05-07-2026 by madpl
 */
#include <GL/glew.h>
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <cstring>
#include <iostream>
#include <memory>
#include <states/MissionState.hpp>
#include <core/App.hpp>
#include <states/MainMenuState.hpp>
#include <core/Event.hpp>
#include <core/EventType.hpp>


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
	m_camera(),
	m_inputSnapshot{},
	m_networkConfig(app.getNetworkConfig()),
	m_slots{},
	m_isHost(false),
	m_sessionReady(false),
	m_localPeerId(0),
	m_localSlotIndex(NetGame::InvalidSlotIndex),
	m_serverTick(0),
	m_nextPeerId(1),
	m_host(),
	m_client(),
	m_latestWorldState{},
	m_hasLatestWorldState(false),
	m_previousAltitude(0.0f),
	m_verticalSpeed(0.0f),
	m_previousRenderState{},
	m_currentRenderState{}
{
	for(auto& slot : m_slots)
		resetInputState(slot.inputState);
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
	
	initializeSlots();
	initializeLocalOwnership();
	
	m_camera.setPerspective(75.0f, 1280.0f / 720.0f, 0.3f, 100000.0f);
	m_camera.updateMatrices();
	
	const Helicopter* observed = getObservedHelicopter();
	if(observed != nullptr)
		m_previousAltitude = observed->getY();
	else
		m_previousAltitude = 0.0f;
	
	m_verticalSpeed = 0.0f;
	
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
	m_client.disconnect();
	m_host.stop();
	m_renderer.destroy();
}


void MissionState::onEvent(Event& event)
{
	switch(event.getType())
	{
		case EventType::QuitRequested:
			m_app.stop();
			event.stopPropagation();
			
			break;
			
		case EventType::WindowResized:
		{
			const WindowResizedEvent& resizedEvent = static_cast<const WindowResizedEvent&>(event);
			
			glViewport(0, 0,
					   static_cast<GLsizei>(resizedEvent.getWidth()),
					   static_cast<GLsizei>(resizedEvent.getHeight()));
			
			const float aspect =
			static_cast<float>(resizedEvent.getWidth()) /
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
	m_previousRenderState = m_currentRenderState;
	
	if(m_networkConfig.mode == NetworkMode::Local)
	{
		if(m_localSlotIndex >= 0 &&
			m_localSlotIndex < static_cast<std::int32_t>(m_slots.size()))
		{
			applyLocalInputToState(m_slots[static_cast<std::size_t>(m_localSlotIndex)].inputState);
			m_slots[static_cast<std::size_t>(m_localSlotIndex)].lastProcessedTick = m_inputSnapshot.tick;
		}
		
		for(auto& slot : m_slots)
		{
			if(not slot.occupied)
				continue;
			
			slot.helicopter.update(dt, m_terrain, slot.inputState);
		}
	}
	
	else if(m_isHost)
	{
		processHostNetworking();
		updateHostSimulation(dt);
	}
	
	else
	{
		processClientNetworking();
		updateClientFromWorldState();
	}
	
	const Helicopter* observed = getObservedHelicopter();
	if(observed != nullptr && dt > 0.0001f)
	{
		m_verticalSpeed = (observed->getY() - m_previousAltitude) / dt;
		m_previousAltitude = observed->getY();
	}
	
	else
		m_verticalSpeed = 0.0f;
	
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


void MissionState::setInputSnapshot(const InputSnapshot& inputSnapshot)
{
	m_inputSnapshot = inputSnapshot;
}


void MissionState::initializeSlots()
{
	for(std::size_t i = 0; i < m_slots.size(); ++i)
	{
		m_slots[i].occupied = false;
		m_slots[i].ownerPeerId = 0;
		m_slots[i].lastProcessedTick = 0;
		resetInputState(m_slots[i].inputState);
		placeSlotHelicopter(i);
	}
}


void MissionState::initializeLocalOwnership()
{
	m_isHost = (m_networkConfig.mode == NetworkMode::Host);
	m_sessionReady = false;
	m_serverTick = 0;
	m_nextPeerId = 1;
	m_hasLatestWorldState = false;
	
	if(m_isHost)
	{
		if(not m_host.start(m_networkConfig.port))
		{
			std::cerr << "Failed to start host on port " << m_networkConfig.port << "\n";
			
			return;
		}
		
		m_localPeerId = NetGame::HostPeerId;
		m_localSlotIndex = 0;
		
		m_slots[0].occupied = true;
		m_slots[0].ownerPeerId = m_localPeerId;
		m_slots[0].lastProcessedTick = 0;
		
		m_sessionReady = true;
	}
	
	else if(m_networkConfig.mode == NetworkMode::Client)
	{
		const sf::IpAddress hostAddress(m_networkConfig.ipAddress);
		
		if(not m_client.connectTo(hostAddress, m_networkConfig.port))
		{
			std::cerr << "Failed to connect to host "
			<< m_networkConfig.ipAddress
			<< ":" << m_networkConfig.port << "\n";
			
			return;
		}
		
		m_client.sendJoinRequest();
		
		JoinAcceptPacket joinAccept{};
		if(m_client.pollJoinAccept(joinAccept) && joinAccept.accepted != 0)
		{
			m_localPeerId = joinAccept.assignedPeerId;
			m_localSlotIndex = joinAccept.assignedSlotIndex;
			m_sessionReady = (m_localSlotIndex != NetGame::InvalidSlotIndex);
		}
		
		else
		{
			m_localPeerId = 0;
			m_localSlotIndex = NetGame::InvalidSlotIndex;
		}
	}
	
	else
	{
		m_localPeerId = NetGame::HostPeerId;
		m_localSlotIndex = 0;
		
		m_slots[0].occupied = true;
		m_slots[0].ownerPeerId = m_localPeerId;
		m_slots[0].lastProcessedTick = 0;
		
		m_sessionReady = true;
	}
}


void MissionState::placeSlotHelicopter(std::size_t slotIndex)
{
	const float spacing = 35.0f;
	const std::size_t row = slotIndex / 2;
	const std::size_t col = slotIndex % 2;
	
	const float x = (static_cast<float>(col) - 0.5f) * spacing * 2.0f;
	const float z = static_cast<float>(row) * spacing * -2.0f;
	const float ground = m_terrain.getHeightAtWorldPosition(x, z);
	
	m_slots[slotIndex].helicopter.setPosition(x, ground + 5.0f, z);
	m_slots[slotIndex].helicopter.setYawDegrees(0.0f);
}


void MissionState::processHostNetworking()
{
	handlePendingJoinRequests();
	handleIncomingPlayerInputs();
}


void MissionState::processClientNetworking()
{
	if(not m_sessionReady)
	{
		m_client.sendJoinRequest();
		
		JoinAcceptPacket joinAccept{};
		if(m_client.pollJoinAccept(joinAccept) && joinAccept.accepted != 0)
		{
			m_localPeerId = joinAccept.assignedPeerId;
			m_localSlotIndex = joinAccept.assignedSlotIndex;
			m_sessionReady = true;
		}
	}
	
	if(not m_sessionReady)
		return;
	
	PlayerInputPacket inputPacket{};
	fillPlayerInputPacket(inputPacket);
	m_client.sendPlayerInput(inputPacket);
	
	WorldStatePacket worldState{};
	
	if(m_client.pollWorldState(worldState))
	{
		m_latestWorldState = worldState;
		m_hasLatestWorldState = true;
	}
}


void MissionState::handlePendingJoinRequests()
{
	for(;;)
	{
		sf::IpAddress address;
		std::uint16_t port = 0;
		
		if(not m_host.pollJoinRequest(address, port))
			break;
		
		std::int32_t slotIndex = NetGame::InvalidSlotIndex;
		JoinAcceptPacket acceptPacket{};
		
		if(findFreeSlotIndex(slotIndex))
		{
			const std::uint32_t peerId = generatePeerId();
			
			if(m_host.registerPeer(address, port, peerId, slotIndex))
			{
				acceptPacket.accepted = 1;
				acceptPacket.assignedPeerId = peerId;
				acceptPacket.assignedSlotIndex = slotIndex;
				
				m_slots[static_cast<std::size_t>(slotIndex)].occupied = true;
				m_slots[static_cast<std::size_t>(slotIndex)].ownerPeerId = peerId;
				m_slots[static_cast<std::size_t>(slotIndex)].lastProcessedTick = 0;
				resetInputState(m_slots[static_cast<std::size_t>(slotIndex)].inputState);
			}
		}
		
		m_host.sendJoinAccept(address, port, acceptPacket);
	}
}


void MissionState::handleIncomingPlayerInputs()
{
	for(;;)
	{
		PlayerInputPacket packet{};
		
		if(not m_host.pollPlayerInput(packet))
			break;
		
		for(auto& slot : m_slots)
		{
			if(not slot.occupied)
				continue;
			
			if(slot.ownerPeerId != packet.peerId)
				continue;
			
			slot.lastProcessedTick = packet.tick;
			slot.inputState.forwardInput = packet.cyclicPitch;
			slot.inputState.yawInput = packet.yawPedal;
			slot.inputState.verticalInput = packet.collective;
			slot.inputState.brake = (packet.brake != 0) || (packet.cyclicPitch < 0.0f);
			
			break;
		}
	}
}


bool MissionState::findFreeSlotIndex(std::int32_t& outSlotIndex) const
{
	for(std::size_t i = 1; i < m_slots.size(); ++i)
	{
		if(not m_slots[i].occupied)
		{
			outSlotIndex = static_cast<std::int32_t>(i);
			return true;
		}
	}
	
	outSlotIndex = NetGame::InvalidSlotIndex;
	
	return false;
}


std::uint32_t MissionState::generatePeerId()
{
	return m_nextPeerId++;
}


void MissionState::updateHostSimulation(float dt)
{
	if(not m_sessionReady)
		return;
	
	if(m_localSlotIndex >= 0 &&
		m_localSlotIndex < static_cast<std::int32_t>(m_slots.size()))
	{
		applyLocalInputToState(m_slots[static_cast<std::size_t>(m_localSlotIndex)].inputState);
		m_slots[static_cast<std::size_t>(m_localSlotIndex)].lastProcessedTick = m_inputSnapshot.tick;
	}
	
	for(auto& slot : m_slots)
	{
		if(not slot.occupied)
			continue;
		
		slot.helicopter.update(dt, m_terrain, slot.inputState);
	}
	
	WorldStatePacket packet{};
	buildWorldStatePacket(packet);
	m_latestWorldState = packet;
	m_hasLatestWorldState = true;
	m_host.sendWorldStateToAll(packet);
	
	++m_serverTick;
}


void MissionState::updateClientFromWorldState()
{
	if(not m_hasLatestWorldState)
		return;
	
	applyWorldStatePacket(m_latestWorldState);
}


void MissionState::buildWorldStatePacket(WorldStatePacket& outPacket) const
{
	outPacket = WorldStatePacket{};
	outPacket.serverTick = m_serverTick;
	outPacket.yourPeerId = m_localPeerId;
	outPacket.yourSlotIndex = m_localSlotIndex;
	outPacket.activePlayerCount = 0;
	
	for(std::size_t i = 0; i < m_slots.size(); ++i)
	{
		writeSlotStateToPacket(i, outPacket.slots[i]);
		
		if(m_slots[i].occupied)
			++outPacket.activePlayerCount;
	}
}


void MissionState::applyWorldStatePacket(const WorldStatePacket& packet)
{
	m_serverTick = packet.serverTick;
	
	if(packet.yourSlotIndex != NetGame::InvalidSlotIndex)
		m_localSlotIndex = packet.yourSlotIndex;
	
	if(packet.yourPeerId != 0)
		m_localPeerId = packet.yourPeerId;
	
	for(const HelicopterSlotStatePacket& slotPacket : packet.slots)
		applyPacketToSlot(slotPacket);
}


void MissionState::writeSlotStateToPacket(std::size_t slotIndex, HelicopterSlotStatePacket& outPacket) const
{
	const HelicopterSlot& slot = m_slots[slotIndex];
	
	outPacket = HelicopterSlotStatePacket{};
	outPacket.slotIndex = static_cast<std::uint8_t>(slotIndex);
	outPacket.occupied = slot.occupied ? static_cast<std::uint8_t>(SlotOccupancy::Occupied)
	: static_cast<std::uint8_t>(SlotOccupancy::Empty);
	outPacket.ownerPeerId = slot.ownerPeerId;
	outPacket.lastProcessedTick = slot.lastProcessedTick;
	
	if(not slot.occupied)
		return;
	
	outPacket.x = slot.helicopter.getX();
	outPacket.y = slot.helicopter.getY();
	outPacket.z = slot.helicopter.getZ();
	outPacket.yawDegrees = slot.helicopter.getYawDegrees();
	outPacket.pitchDegrees = slot.helicopter.getPitchDegrees();
	outPacket.rollDegrees = slot.helicopter.getRollDegrees();
	outPacket.speedMetersPerSecond = slot.helicopter.getSpeed();
	outPacket.verticalSpeedMetersPerSecond = slot.helicopter.getVerticalSpeed();
	outPacket.altitudeAboveGroundMeters = slot.helicopter.getAltitudeAboveGround();
}


void MissionState::applyPacketToSlot(const HelicopterSlotStatePacket& packet)
{
	const std::size_t slotIndex = static_cast<std::size_t>(packet.slotIndex);
	if(slotIndex >= m_slots.size())
		return;
	
	HelicopterSlot& slot = m_slots[slotIndex];
	slot.occupied = (packet.occupied == static_cast<std::uint8_t>(SlotOccupancy::Occupied));
	slot.ownerPeerId = packet.ownerPeerId;
	slot.lastProcessedTick = packet.lastProcessedTick;
	
	if(not slot.occupied)
	{
		resetInputState(slot.inputState);
		placeSlotHelicopter(slotIndex);
		
		return;
	}
	
	slot.helicopter.applyAuthoritativeState(
		packet.x,
		packet.y,
		packet.z,
		packet.yawDegrees,
		packet.pitchDegrees,
		packet.rollDegrees,
		packet.speedMetersPerSecond,
		packet.verticalSpeedMetersPerSecond,
		packet.altitudeAboveGroundMeters);
}


void MissionState::updateHud()
{
	const Helicopter* observed = getObservedHelicopter();
	if(observed == nullptr)
		return;
	
	m_hud.setHeadingDegrees(observed->getYawDegrees());
	m_hud.setAltitudeMeters(observed->getY());
	m_hud.setAltitudeAboveGroundMeters(observed->getAltitudeAboveGround());
	m_hud.setSpeedMetersPerSecond(observed->getSpeed());
	m_hud.setVerticalSpeedMetersPerSecond(m_verticalSpeed);
}


void MissionState::captureCurrentRenderState()
{
	const Helicopter* helicopter = getObservedHelicopter();
	if(helicopter == nullptr)
		return;
	
	const float pi = 3.1415926535f;
	
	const float helicopterX = helicopter->getX();
	const float helicopterY = helicopter->getY();
	const float helicopterZ = helicopter->getZ();
	
	const float helicopterYawDegrees = helicopter->getYawDegrees();
	const float helicopterPitchDegrees = helicopter->getPitchDegrees();
	const float helicopterRollDegrees = helicopter->getRollDegrees();
	
	const float yawRadians = helicopterYawDegrees * pi / 180.0f;
	const float pitchRadians = helicopterPitchDegrees * pi / 180.0f;
	
	const float forwardX = std::sin(yawRadians) * std::cos(pitchRadians);
	const float forwardY = std::sin(pitchRadians);
	const float forwardZ = -std::cos(yawRadians) * std::cos(pitchRadians);
	
	const float cockpitForwardOffset = 1.2f;
	const float cockpitUpOffset = 0.6f;
	const float lookDistance = 250.0f;
	
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
	
	state.helicopterX = lerp(
		m_previousRenderState.helicopterX,
		m_currentRenderState.helicopterX,
		alpha);
	
	state.helicopterY = lerp(
		m_previousRenderState.helicopterY,
		m_currentRenderState.helicopterY,
		alpha);
	
	state.helicopterZ = lerp(
		m_previousRenderState.helicopterZ,
		m_currentRenderState.helicopterZ,
		alpha);
	
	state.helicopterYawDegrees = lerpAngleDegrees(
		m_previousRenderState.helicopterYawDegrees,
		m_currentRenderState.helicopterYawDegrees,
		alpha);
	
	state.helicopterPitchDegrees = lerpAngleDegrees(
		m_previousRenderState.helicopterPitchDegrees,
		m_currentRenderState.helicopterPitchDegrees,
		alpha);
	
	state.helicopterRollDegrees = lerpAngleDegrees(
		m_previousRenderState.helicopterRollDegrees,
		m_currentRenderState.helicopterRollDegrees,
		alpha);
	
	state.camX = lerp(m_previousRenderState.camX, m_currentRenderState.camX, alpha);
	state.camY = lerp(m_previousRenderState.camY, m_currentRenderState.camY, alpha);
	state.camZ = lerp(m_previousRenderState.camZ, m_currentRenderState.camZ, alpha);
	
	state.targetX = lerp(m_previousRenderState.targetX, m_currentRenderState.targetX, alpha);
	state.targetY = lerp(m_previousRenderState.targetY, m_currentRenderState.targetY, alpha);
	state.targetZ = lerp(m_previousRenderState.targetZ, m_currentRenderState.targetZ, alpha);
	
	return state;
}


void MissionState::resetInputState(HelicopterInputState& inputState)
{
	inputState.forwardInput = 0.0f;
	inputState.yawInput = 0.0f;
	inputState.verticalInput = 0.0f;
	inputState.brake = false;
}


void MissionState::applyLocalInputToState(HelicopterInputState& outState) const
{
	outState.forwardInput = m_inputSnapshot.cyclicPitch;
	outState.yawInput = m_inputSnapshot.yawPedal;
	outState.verticalInput = m_inputSnapshot.collective;
	outState.brake = (m_inputSnapshot.cyclicPitch < 0.0f);
}


void MissionState::fillPlayerInputPacket(PlayerInputPacket& outPacket) const
{
	outPacket = PlayerInputPacket{};
	outPacket.peerId = m_localPeerId;
	outPacket.tick = m_inputSnapshot.tick;
	outPacket.collective = m_inputSnapshot.collective;
	outPacket.cyclicPitch = m_inputSnapshot.cyclicPitch;
	outPacket.cyclicRoll = m_inputSnapshot.cyclicRoll;
	outPacket.yawPedal = m_inputSnapshot.yawPedal;
	outPacket.brake = (m_inputSnapshot.cyclicPitch < 0.0f) ? 1 : 0;
	outPacket.fireCannon = m_inputSnapshot.fireCannon ? 1 : 0;
	outPacket.launchMissile = m_inputSnapshot.launchMissile ? 1 : 0;
	outPacket.pauseRequested = m_inputSnapshot.pauseRequested ? 1 : 0;
}


std::int32_t MissionState::getObservedSlotIndex() const
{
	if(m_localSlotIndex != NetGame::InvalidSlotIndex)
		return m_localSlotIndex;
	
	for(std::size_t i = 0; i < m_slots.size(); ++i)
	{
		if(m_slots[i].occupied)
			return static_cast<std::int32_t>(i);
	}
	
	return NetGame::InvalidSlotIndex;
}


const Helicopter* MissionState::getObservedHelicopter() const
{
	const std::int32_t slotIndex = getObservedSlotIndex();
	if(slotIndex == NetGame::InvalidSlotIndex)
		return nullptr;
	
	return &m_slots[static_cast<std::size_t>(slotIndex)].helicopter;
}
