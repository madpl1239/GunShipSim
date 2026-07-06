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
	m_authoritativeYawDegrees(0.0f),
	m_authoritativePitchDegrees(0.0f),
	m_authoritativeRollDegrees(0.0f),
	m_authoritativeSpeed(0.0f),
	m_authoritativeVerticalSpeed(0.0f)
{
	// no-op
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
	m_authoritativeYawDegrees = yawDegrees;
}


void Helicopter::update(float dt, const TerrainData& terrain)
{
	const HelicopterInputState inputState = m_input.readInput();
	update(dt, terrain, inputState);
}


void Helicopter::update(float dt, const TerrainData& terrain, const HelicopterInputState& inputState)
{
	m_flightModel.update(dt, inputState);
	updateMovement(dt);
	updateTerrainRelation(terrain);
	
	m_authoritativeYawDegrees = m_flightModel.getYawDegrees();
	m_authoritativePitchDegrees = m_flightModel.getPitchDegrees();
	m_authoritativeRollDegrees = m_flightModel.getRollDegrees();
	m_authoritativeSpeed = m_flightModel.getSpeed();
	m_authoritativeVerticalSpeed = m_flightModel.getVerticalSpeed();
}


void Helicopter::applyAuthoritativeState(
	float x,
	float y,
	float z,
	float yawDegrees,
	float pitchDegrees,
	float rollDegrees,
	float speed,
	float verticalSpeed,
	float altitudeAboveGround)
{
	m_x = x;
	m_y = y;
	m_z = z;
	
	m_authoritativeYawDegrees = yawDegrees;
	m_authoritativePitchDegrees = pitchDegrees;
	m_authoritativeRollDegrees = rollDegrees;
	m_authoritativeSpeed = speed;
	m_authoritativeVerticalSpeed = verticalSpeed;
	m_altitudeAboveGround = altitudeAboveGround;
	
	m_flightModel.setYawDegrees(yawDegrees);
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
	return m_authoritativeYawDegrees;
}


float Helicopter::getPitchDegrees() const
{
	return m_authoritativePitchDegrees;
}


float Helicopter::getRollDegrees() const
{
	return m_authoritativeRollDegrees;
}


float Helicopter::getSpeed() const
{
	return m_authoritativeSpeed;
}


float Helicopter::getVerticalSpeed() const
{
	return m_authoritativeVerticalSpeed;
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
	const float terrainHeight = terrain.getHeightAtWorldPosition(m_x, m_z);
	
	if(m_y < terrainHeight + 5.0f)
		m_y = terrainHeight + 5.0f;
	
	m_altitudeAboveGround = m_y - terrainHeight;
}
