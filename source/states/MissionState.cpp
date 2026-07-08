/*
 * MissionState.cpp
 *
 * 05-07-2026 by madpl
 */
#include <GL/glew.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>
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
	
	float toMilliseconds(float seconds)
	{
		return seconds * 1000.0f;
	}
}


MissionState::MissionState(StateManager& manager, App& app):
	IState(manager),
	m_app(app),
	m_hud(),
	m_terrain(),
	m_renderer(),
	m_helicopterRenderer(),
	m_helicopter(),
	m_camera(),
	m_inputState{},
	m_inputSnapshot{},
	m_networkConfig(app.getNetworkConfig()),
	m_statusFont(),
	m_statusText(),
	m_statusBackground(),
	m_previousAltitude(0.0f),
	m_verticalSpeed(0.0f),
	m_previousRenderState{},
	m_currentRenderState{},
	m_joinRequestSent(false),
	m_nextPeerId(1),
	m_slots{},
	m_lastWorldState{},
	m_hasWorldState(false),
	m_lastAcceptedWorldStateTick(0),
	m_showDebugOverlay(true),
	m_freezeDebugOverlay(false),
	m_snapshotMode(false),
	m_debugTextRefreshAccum(0.0f),
	m_debugTextRefreshInterval(0.25f),
	m_frozenDebugText(),
	m_debugNetAccumSeconds(0.0f),
	m_debugClientSentInputTotal(0),
	m_debugClientSentInputPerSecond(0),
	m_debugClientSentInputThisWindow(0),
	m_debugClientLastSendDt(-1.0f),
	m_debugClientMaxSendDtInWindow(0.0f),
	m_debugHostReceivedInputTotal(0),
	m_debugHostReceivedInputPerSecond(0),
	m_debugHostReceivedInputThisWindow(0),
	m_debugHostLastInputPeerId(0),
	m_debugHostLastInputTick(0),
	m_debugHostLastReceiveDt(-1.0f),
	m_debugHostMaxReceiveDtInWindow(0.0f),
	m_debugClientReceivedWorldTotal(0),
	m_debugClientReceivedWorldPerSecond(0),
	m_debugClientReceivedWorldThisWindow(0),
	m_debugClientDroppedStaleWorldTotal(0),
	m_debugClientDroppedStaleWorldThisWindow(0),
	m_debugClientLastWorldStateTick(0),
	m_debugClientLastWorldReceiveDt(-1.0f),
	m_debugClientMaxWorldReceiveDtInWindow(0.0f)
{
	resetInputState();
}


MissionState::~MissionState()
{
	m_helicopterRenderer.destroy();
	m_renderer.destroy();
}


void MissionState::onEnter()
{
	if(not m_hud.initialize("fonts/DejaVuSans.ttf"))
	{
		std::cerr << "Failed to load HUD font\n";
		
		return;
	}
	
	initializeUi();
	
	if(not initializeTerrainAndRenderer())
		return;
	
	if(not m_helicopterRenderer.create())
	{
		std::cerr << "Failed to create helicopter renderer\n";
		
		return;
	}
	
	initializeScene();
	initializeNetworkSlots();
	resetNetworkDebugCounters();
	
	m_lastAcceptedWorldStateTick = 0;
	
	captureCurrentRenderState();
	m_previousRenderState = m_currentRenderState;
	
	initializeGraphicsState();
	refreshDebugText();
	updateHud();
}


void MissionState::onExit()
{
	m_renderer.destroy();
	
	if(m_networkConfig.mode == NetworkMode::Host)
		m_app.getNetHost().stop();
	
	else if(m_networkConfig.mode == NetworkMode::Client)
		m_app.getNetClient().disconnect();
	
	m_helicopterRenderer.destroy();
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
			handleWindowResizeEvent(event);
			event.stopPropagation();
			
			break;
		}
		
		case EventType::KeyPressed:
		{
			handleKeyPressedEvent(event);
			
			break;
		}
		
		default:
			break;
	}
}


void MissionState::update(float dt)
{
	updateNetworking(dt);
	
	m_previousRenderState = m_currentRenderState;
	updateLocalSimulation(dt);
	publishLocalHelicopterToOwnedSlot();
	captureCurrentRenderState();
	
	updateNetworkDebugWindow(dt);
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
	
	renderNetworkHelicopters();
	
	m_app.getWindow().pushGLStates();
	renderHud();
	renderDebugOverlay();
	m_app.getWindow().popGLStates();
}


void MissionState::setInputSnapshot(const InputSnapshot& inputSnapshot)
{
	m_inputSnapshot = inputSnapshot;
}


void MissionState::initializeUi()
{
	if(not m_statusFont.loadFromFile("fonts/DejaVuSans.ttf"))
	{
		std::cerr << "Failed to load status font\n";
		
		return;
	}
	
	m_statusText.setFont(m_statusFont);
	m_statusText.setCharacterSize(20);
	m_statusText.setFillColor(sf::Color::White);
	m_statusText.setPosition(20.0f, 18.0f);
	
	m_statusBackground.setPosition(10.0f, 10.0f);
	m_statusBackground.setSize(sf::Vector2f(560.0f, 300.0f));
	m_statusBackground.setFillColor(sf::Color(0, 0, 0, 170));
	m_statusBackground.setOutlineThickness(1.0f);
	m_statusBackground.setOutlineColor(sf::Color(120, 120, 120, 220));
}


bool MissionState::initializeTerrainAndRenderer()
{
	HGTLoader loader;
	HGTLoader::Data rawData;
	
	if(not loader.load("res/terrain/N34E062.hgt", rawData))
	{
		std::cerr << "Failed to load HGT file\n";
		
		return false;
	}
	
	if(not m_terrain.buildFromHGT(rawData.samples, rawData.width, rawData.height,
									34.0f, 62.0f, 34.5f, 62.5f, 12000.0f, 256))
	{
		std::cerr << "Failed to build terrain data\n";
		
		return false;
	}
	
	if(not m_renderer.create(m_terrain))
	{
		std::cerr << "Failed to create terrain renderer\n";
		
		return false;
	}
	
	return true;
}


void MissionState::initializeScene()
{
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
		{
			std::cerr << "Failed to connect client socket\n";
		}
	}
	
	m_helicopter.setPosition(0.0f, m_terrain.getHeightAtWorldPosition(0.0f, 0.0f) + 5.0f, 0.0f);
	m_helicopter.setYawDegrees(0.0f);
	
	m_camera.setPerspective(75.0f, 1280.0f / 720.0f, 0.3f, 100000.0f);
	m_camera.updateMatrices();
	
	m_previousAltitude = m_helicopter.getY();
	m_verticalSpeed = 0.0f;
}


void MissionState::initializeGraphicsState()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearDepth(1.0f);
	glClearColor(0.60f, 0.75f, 0.95f, 1.0f);
}


void MissionState::handleWindowResizeEvent(const Event& event)
{
	const WindowResizedEvent& resizedEvent = static_cast<const WindowResizedEvent&>(event);
	
	glViewport(0, 0, static_cast<GLsizei>(resizedEvent.getWidth()),
			   static_cast<GLsizei>(resizedEvent.getHeight()));
	
	float aspect = static_cast<float>(resizedEvent.getWidth()) /
					static_cast<float>(resizedEvent.getHeight());
	
	m_camera.setPerspective(75.0f, aspect, 0.3f, 100000.0f);
}


void MissionState::handleKeyPressedEvent(Event& event)
{
	KeyEvent& keyEvent = static_cast<KeyEvent&>(event);
	
	if(keyEvent.getKey() == sf::Keyboard::Escape)
	{
		m_manager.replaceState(std::make_unique<MainMenuState>(m_manager, m_app));
		event.stopPropagation();
	}
	
	else if(keyEvent.getKey() == sf::Keyboard::F3)
	{
		m_showDebugOverlay = not m_showDebugOverlay;
		event.stopPropagation();
	}
	
	else if(keyEvent.getKey() == sf::Keyboard::F4)
	{
		m_freezeDebugOverlay = not m_freezeDebugOverlay;
		
		if(m_freezeDebugOverlay)
		{
			m_snapshotMode = false;
			captureDebugSnapshot();
		}
		
		event.stopPropagation();
	}
	
	else if(keyEvent.getKey() == sf::Keyboard::F5)
	{
		m_snapshotMode = true;
		m_freezeDebugOverlay = false;
		captureDebugSnapshot();
		event.stopPropagation();
	}
}


void MissionState::updateNetworking(float dt)
{
	if(m_networkConfig.mode == NetworkMode::Host)
	{
		m_app.getNetHost().pumpIncomingPackets();
		updateHostNetworking(dt);
	}
	
	else if(m_networkConfig.mode == NetworkMode::Client)
	{
		m_app.getNetClient().pumpIncomingPackets();
		updateClientJoinAccept();
		updateClientNetworking();
	}
}


void MissionState::updateClientJoinAccept()
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
}


void MissionState::updateLocalSimulation(float dt)
{
	applyInputSnapshot();
	m_helicopter.update(dt, m_terrain);
	
	if(dt > 0.0001f)
		m_verticalSpeed = (m_helicopter.getY() - m_previousAltitude) / dt;
	
	m_previousAltitude = m_helicopter.getY();
}


void MissionState::updateHud()
{
	m_hud.setHeadingDegrees(m_helicopter.getYawDegrees());
	m_hud.setAltitudeMeters(m_helicopter.getY());
	m_hud.setAltitudeAboveGroundMeters(m_helicopter.getAltitudeAboveGround());
	m_hud.setSpeedMetersPerSecond(m_helicopter.getSpeed());
	m_hud.setVerticalSpeedMetersPerSecond(m_verticalSpeed);
}

void MissionState::renderHud()
{
	m_hud.draw(m_app.getWindow());
}


void MissionState::renderDebugOverlay()
{
	if(not m_showDebugOverlay)
		return;
	
	m_app.getWindow().draw(m_statusBackground);
	m_app.getWindow().draw(m_statusText);
}


void MissionState::captureDebugSnapshot()
{
	refreshDebugText();
	m_frozenDebugText = m_statusText.getString();
}


std::string MissionState::buildDebugText() const
{
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(1);
	
	if(m_freezeDebugOverlay)
		oss << "[F3] show/hide  [F4] unfreeze  [F5] snapshot\nstate: FROZEN\n";
	
	else if(m_snapshotMode)
		oss << "[F3] show/hide  [F4] freeze live  [F5] snapshot\nstate: SNAPSHOT\n";
	
	else
		oss << "[F3] show/hide  [F4] freeze live  [F5] snapshot\nstate: LIVE\n";
	
	if(m_networkConfig.mode == NetworkMode::Host)
		appendHostDebugText(oss);
	
	else if(m_networkConfig.mode == NetworkMode::Client)
		appendClientDebugText(oss);
	
	else
		appendLocalDebugText(oss);
	
	return oss.str();
}


void MissionState::appendHostDebugText(std::ostringstream& oss) const
{
	const std::string& msg = m_app.getNetHost().getLastStatusMessage();
	
	oss << "mode: HOST\n"
		<< "status: " << (msg.empty() ? "Hosting session" : msg) << "\n\n"
		<< "input RX total: " << m_debugHostReceivedInputTotal << "\n"
		<< "input RX/sec : " << m_debugHostReceivedInputPerSecond << "\n"
		<< "last peerId  : " << m_debugHostLastInputPeerId << "\n"
		<< "last tick    : " << m_debugHostLastInputTick << "\n"
		<< "last recv dt : ";
	
	if(m_debugHostLastReceiveDt < 0.0f)
		oss << "n/a";
	else
		oss << toMilliseconds(m_debugHostLastReceiveDt) << " ms";
	
	oss << "\nmax recv dt  : " << toMilliseconds(m_debugHostMaxReceiveDtInWindow) << " ms";
}


void MissionState::appendClientDebugText(std::ostringstream& oss) const
{
	const std::string& msg = m_app.getNetClient().getLastStatusMessage();
	
	oss << "mode: CLIENT\n"
		<< "status: " << (msg.empty() ? "Connecting to host..." : msg) << "\n"
		<< "accepted: " << (m_app.getNetClient().isAccepted() ? "yes" : "no") << "\n"
		<< "slot : " << m_app.getNetClient().getAssignedSlotIndex() << "\n\n"
		<< "input TX total : " << m_debugClientSentInputTotal << "\n"
		<< "input TX/sec : " << m_debugClientSentInputPerSecond << "\n"
		<< "last send dt : ";
	
	if(m_debugClientLastSendDt < 0.0f)
		oss << "n/a";
	else
		oss << toMilliseconds(m_debugClientLastSendDt) << " ms";
	
	oss << "\nmax send dt : " << toMilliseconds(m_debugClientMaxSendDtInWindow) << " ms"
		<< "\n\nworld RX total : " << m_debugClientReceivedWorldTotal
		<< "\nworld RX/sec : " << m_debugClientReceivedWorldPerSecond
		<< "\nstale dropped : " << m_debugClientDroppedStaleWorldTotal
		<< "\nlast world tick: " << m_debugClientLastWorldStateTick
		<< "\nlast world dt : ";
	
	if(m_debugClientLastWorldReceiveDt < 0.0f)
		oss << "n/a";
	else
		oss << toMilliseconds(m_debugClientLastWorldReceiveDt) << " ms";
	
	oss << "\nmax world dt   : " << toMilliseconds(m_debugClientMaxWorldReceiveDtInWindow) << " ms";
}


void MissionState::appendLocalDebugText(std::ostringstream& oss) const
{
	oss << "mode: LOCAL\nstatus: local session";
}


void MissionState::refreshDebugText()
{
	const std::string liveText = buildDebugText();
	
	if(m_freezeDebugOverlay || m_snapshotMode)
	{
		if(not m_frozenDebugText.empty())
			m_statusText.setString(m_frozenDebugText);
		
		else
			m_statusText.setString(liveText);
	}
	
	else
	{
		m_statusText.setString(liveText);
	}
	
	const sf::FloatRect bounds = m_statusText.getLocalBounds();
	m_statusBackground.setSize(sf::Vector2f(bounds.width + 32.0f, bounds.height + 28.0f));
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
	state.camY = lerp(m_previousRenderState.camY, m_currentRenderState.camY, alpha);
	state.camZ = lerp(m_previousRenderState.camZ, m_currentRenderState.camZ, alpha);
	
	state.targetX = lerp(m_previousRenderState.targetX, m_currentRenderState.targetX, alpha);
	state.targetY = lerp(m_previousRenderState.targetY, m_currentRenderState.targetY, alpha);
	state.targetZ = lerp(m_previousRenderState.targetZ, m_currentRenderState.targetZ, alpha);
	
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
	
	if(m_networkConfig.mode == NetworkMode::Host || m_networkConfig.mode == NetworkMode::Local)
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
	
	slot.helicopter.update(dt, m_terrain, inputState);
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
		out.verticalSpeedMetersPerSecond = slot.helicopter.getVerticalSpeed();
		out.altitudeAboveGroundMeters = slot.helicopter.getAltitudeAboveGround();
		
		if(out.occupied != 0)
			++packet.activePlayerCount;
	}
}


void MissionState::updateHostNetworking(float dt)
{
	sf::IpAddress joinAddress;
	std::uint16_t joinPort = 0;
	
	while(m_app.getNetHost().pollJoinRequest(joinAddress, joinPort))
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
		++m_debugHostReceivedInputTotal;
		++m_debugHostReceivedInputThisWindow;
		m_debugHostLastInputPeerId = inputPacket.peerId;
		m_debugHostLastInputTick = inputPacket.tick;
		m_debugHostLastReceiveDt = 0.0f;
		
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
	
	/*
	m_slots[0].occupancy = SlotOccupancy::Occupied;
	m_slots[0].ownerPeerId = NetGame::HostPeerId;
	m_slots[0].helicopter = m_helicopter;
	m_slots[0].lastProcessedTick = static_cast<std::uint32_t>(m_inputSnapshot.tick);
	*/
	
	WorldStatePacket packet{};
	fillWorldStatePacket(packet);
	m_app.getNetHost().sendWorldStateToAll(packet);
}


bool MissionState::applyWorldStatePacket(const WorldStatePacket& packet)
{
	if(m_hasWorldState and packet.serverTick < m_lastAcceptedWorldStateTick)
	{
		++m_debugClientDroppedStaleWorldTotal;
		++m_debugClientDroppedStaleWorldThisWindow;
		
		return false;
	}
	
	m_lastWorldState = packet;
	m_hasWorldState = true;
	m_lastAcceptedWorldStateTick = packet.serverTick;
	
	const std::int32_t localClientSlot = m_app.getNetClient().getAssignedSlotIndex();
	
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
		
		if(m_networkConfig.mode == NetworkMode::Client &&
			static_cast<std::int32_t>(i) == localClientSlot)
		{
			continue;
		}
		
		slot.helicopter.applyAuthoritativeState(in.x, in.y, in.z, in.yawDegrees, in.pitchDegrees,
												in.rollDegrees, in.speedMetersPerSecond,
												in.verticalSpeedMetersPerSecond,
												in.altitudeAboveGroundMeters);
	}
	
	return true;
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
		
		if(m_app.getNetClient().sendPlayerInput(packet))
		{
			++m_debugClientSentInputTotal;
			++m_debugClientSentInputThisWindow;
			m_debugClientLastSendDt = 0.0f;
		}
	}

	WorldStatePacket worldPacket{};
	while(m_app.getNetClient().pollWorldState(worldPacket))
	{
		m_debugClientLastWorldReceiveDt = 0.0f;
		
		if(applyWorldStatePacket(worldPacket))
		{
			++m_debugClientReceivedWorldTotal;
			++m_debugClientReceivedWorldThisWindow;
			m_debugClientLastWorldStateTick = worldPacket.serverTick;
		}
	}
}


void MissionState::updateNetworkDebugWindow(float dt)
{
	if(m_debugClientLastSendDt >= 0.0f)
	{
		m_debugClientLastSendDt += dt;
		m_debugClientMaxSendDtInWindow = std::max(m_debugClientMaxSendDtInWindow, m_debugClientLastSendDt);
	}
	
	if(m_debugHostLastReceiveDt >= 0.0f)
	{
		m_debugHostLastReceiveDt += dt;
		m_debugHostMaxReceiveDtInWindow = std::max(m_debugHostMaxReceiveDtInWindow, m_debugHostLastReceiveDt);
	}
	
	if(m_debugClientLastWorldReceiveDt >= 0.0f)
	{
		m_debugClientLastWorldReceiveDt += dt;
		m_debugClientMaxWorldReceiveDtInWindow = std::max(m_debugClientMaxWorldReceiveDtInWindow, m_debugClientLastWorldReceiveDt);
	}
	
	m_debugNetAccumSeconds += dt;
	m_debugTextRefreshAccum += dt;
	
	if(not m_freezeDebugOverlay && not m_snapshotMode && m_debugTextRefreshAccum >= m_debugTextRefreshInterval)
	{
		refreshDebugText();
		m_debugTextRefreshAccum = 0.0f;
	}
	
	if(m_debugNetAccumSeconds >= 1.0f)
	{
		m_debugClientSentInputPerSecond = m_debugClientSentInputThisWindow;
		m_debugHostReceivedInputPerSecond = m_debugHostReceivedInputThisWindow;
		m_debugClientReceivedWorldPerSecond = m_debugClientReceivedWorldThisWindow;
		
		m_debugClientSentInputThisWindow = 0;
		m_debugHostReceivedInputThisWindow = 0;
		m_debugClientReceivedWorldThisWindow = 0;
		
		m_debugClientMaxSendDtInWindow = 0.0f;
		m_debugHostMaxReceiveDtInWindow = 0.0f;
		m_debugClientMaxWorldReceiveDtInWindow = 0.0f;
		
		m_debugNetAccumSeconds -= 1.0f;
	}
}


void MissionState::resetNetworkDebugCounters()
{
	m_debugTextRefreshAccum = 0.0f;
	m_debugNetAccumSeconds = 0.0f;
	m_frozenDebugText.clear();
	
	m_debugClientSentInputTotal = 0;
	m_debugClientSentInputPerSecond = 0;
	m_debugClientSentInputThisWindow = 0;
	m_debugClientLastSendDt = -1.0f;
	m_debugClientMaxSendDtInWindow = 0.0f;
	
	m_debugHostReceivedInputTotal = 0;
	m_debugHostReceivedInputPerSecond = 0;
	m_debugHostReceivedInputThisWindow = 0;
	m_debugHostLastInputPeerId = 0;
	m_debugHostLastInputTick = 0;
	m_debugHostLastReceiveDt = -1.0f;
	m_debugHostMaxReceiveDtInWindow = 0.0f;

	m_debugClientReceivedWorldTotal = 0;
	m_debugClientReceivedWorldPerSecond = 0;
	m_debugClientReceivedWorldThisWindow = 0;
	m_debugClientDroppedStaleWorldTotal = 0;
	m_debugClientDroppedStaleWorldThisWindow = 0;
	m_debugClientLastWorldStateTick = 0;
	m_debugClientLastWorldReceiveDt = -1.0f;
	m_debugClientMaxWorldReceiveDtInWindow = 0.0f;
}


void MissionState::renderNetworkHelicopters()
{
	const glm::mat4 viewProjectionMatrix = m_camera.getViewProjectionMatrix();
	
	std::uint32_t localPeerId = NetGame::HostPeerId;
	
	if(m_networkConfig.mode == NetworkMode::Client)
		localPeerId = m_app.getNetClient().getAssignedPeerId();
	
	for(std::size_t i = 0; i < m_slots.size(); ++i)
	{
		const HelicopterSlot& slot = m_slots[i];
		
		if(slot.occupancy != SlotOccupancy::Occupied)
			continue;
		
		if(slot.ownerPeerId == localPeerId)
			continue;
		
		glm::vec3 color(0.85f, 0.25f, 0.25f);
		
		if(slot.ownerPeerId == NetGame::HostPeerId)
			color = glm::vec3(0.95f, 0.80f, 0.20f);
		
		m_helicopterRenderer.renderBoxModel(viewProjectionMatrix, slot.helicopter.getX(),
											slot.helicopter.getY() + 1.2f, slot.helicopter.getZ(),
											slot.helicopter.getYawDegrees(),
											slot.helicopter.getPitchDegrees(),
											slot.helicopter.getRollDegrees(), color);
	}
}


void MissionState::publishLocalHelicopterToOwnedSlot()
{
	if(m_networkConfig.mode == NetworkMode::Host or m_networkConfig.mode == NetworkMode::Local)
	{
		m_slots[0].occupancy = SlotOccupancy::Occupied;
		m_slots[0].locallyControlled = true;
		m_slots[0].ownerPeerId = NetGame::HostPeerId;
		m_slots[0].helicopter = m_helicopter;
		m_slots[0].lastProcessedTick = static_cast<std::uint32_t>(m_inputSnapshot.tick);
		
		return;
	}
	
	if(m_networkConfig.mode == NetworkMode::Client)
	{
		const std::int32_t localSlotIndex = m_app.getNetClient().getAssignedSlotIndex();
		
		if(localSlotIndex < 0 or localSlotIndex >= static_cast<std::int32_t>(m_slots.size()))
			return;
		
		HelicopterSlot& localSlot = m_slots[localSlotIndex];
		localSlot.occupancy = SlotOccupancy::Occupied;
		localSlot.locallyControlled = true;
		localSlot.ownerPeerId = m_app.getNetClient().getAssignedPeerId();
		localSlot.helicopter = m_helicopter;
		localSlot.lastProcessedTick = static_cast<std::uint32_t>(m_inputSnapshot.tick);
	}
}
