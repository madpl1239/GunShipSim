/*
 * Helicopter.cpp
 *
 * 02-07-2026 by madpl
 */
#include <cmath>
#include <helicopter/Helicopter.hpp>


Helicopter::Helicopter():
	m_x(0.0f),
	m_y(200.0f),
	m_z(0.0f),
	m_altitudeAboveGround(0.0f),
	m_flightModel(),
	m_input(),
	m_hasNetworkInputOverride(false),
	m_networkInputState{0.0f, 0.0f, 0.0f, false}
{
	// empty
}


void Helicopter::setPosition(float x, float y, float z)
{
	m_x = x;
	m_y = y;
	m_z = z;
}


void Helicopter::setYawDegrees(float yawDegrees)
{
	m_flightModel.setYawDegrees(yawDegrees);
}


void Helicopter::setNetworkInputState(const HelicopterInputState& inputState)
{
	m_networkInputState = inputState;
	m_hasNetworkInputOverride = true;
}


void Helicopter::clearNetworkInputOverride()
{
	m_hasNetworkInputOverride = false;
}


void Helicopter::update(float dt, const TerrainData& terrain)
{
	HelicopterInputState inputState{};
	
	if(m_hasNetworkInputOverride)
		inputState = m_networkInputState;
	else
		inputState = m_input.readInput();
	
	m_flightModel.update(dt, inputState);
	updateMovement(dt);
	updateTerrainRelation(terrain);
}


float Helicopter::getX() const
{
	return m_x;
}


float Helicopter::getY() const
{
	return m_y;
}


float Helicopter::getZ() const
{
	return m_z;
}


float Helicopter::getYawDegrees() const
{
	return m_flightModel.getYawDegrees();
}


float Helicopter::getPitchDegrees() const
{
	return m_flightModel.getPitchDegrees();
}


float Helicopter::getRollDegrees() const
{
	return m_flightModel.getRollDegrees();
}


float Helicopter::getSpeed() const
{
	return m_flightModel.getSpeed();
}


float Helicopter::getVerticalSpeed() const
{
	return m_flightModel.getVerticalSpeed();
}


float Helicopter::getAltitudeAboveGround() const
{
	return m_altitudeAboveGround;
}


void Helicopter::updateMovement(float dt)
{
	const float pi = 3.1415926535f;
	const float yawRadians = m_flightModel.getYawDegrees() * pi / 180.0f;
	
	const float forwardX = std::sin(yawRadians);
	const float forwardZ = -std::cos(yawRadians);
	
	m_x += forwardX * m_flightModel.getSpeed() * dt;
	m_z += forwardZ * m_flightModel.getSpeed() * dt;
	m_y += m_flightModel.getVerticalSpeed() * dt;
}


void Helicopter::updateTerrainRelation(const TerrainData& terrain)
{
	float terrainHeight = terrain.getHeightAtWorldPosition(m_x, m_z);
	
	if(m_y < terrainHeight + 5.0f)
		m_y = terrainHeight + 5.0f;
	
	m_altitudeAboveGround = m_y - terrainHeight;
}
