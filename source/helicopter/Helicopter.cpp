/*
 * Helicopter.cpp
 *
 * 02-07-2026 by madpl
 */
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <algorithm>
#include <terrain/TerrainData.hpp>
#include <helicopter/Helicopter.hpp>


Helicopter::Helicopter():
	m_x(0.0f),
	m_y(200.0f),
	m_z(0.0f),
	m_yawDegrees(0.0f),
	m_pitchDegrees(0.0f),
	m_rollDegrees(0.0f),
	m_speed(0.0f),
	m_verticalSpeed(0.0f),
	m_altitudeAboveGround(0.0f),
	m_maxForwardSpeed(50.0f),
	m_acceleration(40.0f),
	m_deceleration(30.0f),
	m_turnSpeed(60.0f),
	m_climbSpeed(35.0f),
	m_targetPitchDegrees(0.0f),
	m_targetRollDegrees(0.0f)
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
	m_yawDegrees = yawDegrees;
}


void Helicopter::update(float dt, const TerrainData &terrain)
{
	handleInput(dt);
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
	return m_yawDegrees;
}


float Helicopter::getPitchDegrees() const
{
	return m_pitchDegrees;
}


float Helicopter::getRollDegrees() const
{
	return m_rollDegrees;
}


float Helicopter::getSpeed() const
{
	return m_speed;
}


float Helicopter::getVerticalSpeed() const
{
	return m_verticalSpeed;
}


float Helicopter::getAltitudeAboveGround() const
{
	return m_altitudeAboveGround;
}


void Helicopter::handleInput(float dt)
{
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		m_speed += m_acceleration * dt;
	
	else if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		m_speed -= m_deceleration * dt;
	
	else if(m_speed > 0.0f)
		m_speed -= m_deceleration * 0.5f * dt;
	
	if(m_speed < 0.0f)
		m_speed = 0.0f;
	
	m_speed = std::clamp(m_speed, 0.0f, m_maxForwardSpeed);
	
	float yawInput = 0.0f;
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		yawInput -= 1.0f;
		m_yawDegrees -= m_turnSpeed * dt;
	}
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		yawInput += 1.0f;
		m_yawDegrees += m_turnSpeed * dt;
	}
	
	m_verticalSpeed = 0.0f;
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
		m_verticalSpeed = m_climbSpeed;
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::E))
		m_verticalSpeed = -m_climbSpeed;
	
	float speedRatio = 0.0f;
	
	if(m_maxForwardSpeed > 0.001f)
		speedRatio = m_speed / m_maxForwardSpeed;
	
	m_targetPitchDegrees = -12.0f * speedRatio;
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		m_targetPitchDegrees += 6.0f;
	
	m_targetRollDegrees = -25.0f * yawInput * (0.35f + speedRatio * 0.65f);
}


void Helicopter::updateMovement(float dt)
{
	const float pi = 3.1415926535f;
	const float yawRadians = m_yawDegrees * pi / 180.0f;
	
	const float forwardX = std::sin(yawRadians);
	const float forwardZ = -std::cos(yawRadians);
	
	m_x += forwardX * m_speed * dt;
	m_z += forwardZ * m_speed * dt;
	m_y += m_verticalSpeed * dt;
	
	const float pitchResponse = 5.0f;
	const float rollResponse = 6.0f;
	
	m_pitchDegrees += (m_targetPitchDegrees - m_pitchDegrees) * std::min(1.0f, pitchResponse * dt);
	m_rollDegrees += (m_targetRollDegrees - m_rollDegrees) * std::min(1.0f, rollResponse * dt);
}


void Helicopter::updateTerrainRelation(const TerrainData &terrain)
{
	float terrainHeight = terrain.getHeightAtWorldPosition(m_x, m_z);
	
	if(m_y < terrainHeight + 5.0f)
		m_y = terrainHeight + 5.0f;
	
	m_altitudeAboveGround = m_y - terrainHeight;
}
