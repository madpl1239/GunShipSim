/*
 * FlightModel.cpp
 * 
 * 04-07-2026 by madpl
 */
#include <algorithm>
#include <helicopter/FlightModel.hpp>


FlightModel::FlightModel():
m_yawDegrees(0.0f),
m_pitchDegrees(0.0f),
m_rollDegrees(0.0f),
m_speed(0.0f),
m_verticalSpeed(0.0f),
m_maxForwardSpeed(50.0f),
m_acceleration(40.0f),
m_deceleration(30.0f),
m_turnSpeed(60.0f),
m_climbSpeed(35.0f),
m_targetPitchDegrees(0.0f),
m_targetRollDegrees(0.0f)
{
}


void FlightModel::setYawDegrees(float yawDegrees)
{
	m_yawDegrees = yawDegrees;
}


void FlightModel::update(float dt, const HelicopterInputState &inputState)
{
	if(inputState.forwardInput > 0.0f)
	{
		m_speed += m_acceleration * dt;
	}
	else if(inputState.forwardInput < 0.0f)
	{
		m_speed -= m_deceleration * dt;
	}
	else if(m_speed > 0.0f)
	{
		m_speed -= m_deceleration * 0.5f * dt;
	}
	
	if(m_speed < 0.0f)
	{
		m_speed = 0.0f;
	}
	
	m_speed = std::clamp(m_speed, 0.0f, m_maxForwardSpeed);
	
	m_yawDegrees += inputState.yawInput * m_turnSpeed * dt;
	
	m_verticalSpeed = inputState.verticalInput * m_climbSpeed;
	
	float speedRatio = 0.0f;
	
	if(m_maxForwardSpeed > 0.001f)
	{
		speedRatio = m_speed / m_maxForwardSpeed;
	}
	
	m_targetPitchDegrees = -12.0f * speedRatio;
	
	if(inputState.brake)
	{
		m_targetPitchDegrees += 6.0f;
	}
	
	m_targetRollDegrees = -25.0f * inputState.yawInput * (0.35f + speedRatio * 0.65f);
	
	const float pitchResponse = 5.0f;
	const float rollResponse = 6.0f;
	
	m_pitchDegrees += (m_targetPitchDegrees - m_pitchDegrees) * std::min(1.0f, pitchResponse * dt);
	m_rollDegrees += (m_targetRollDegrees - m_rollDegrees) * std::min(1.0f, rollResponse * dt);
}


float FlightModel::getYawDegrees() const
{
	return m_yawDegrees;
}


float FlightModel::getPitchDegrees() const
{
	return m_pitchDegrees;
}


float FlightModel::getRollDegrees() const
{
	return m_rollDegrees;
}


float FlightModel::getSpeed() const
{
	return m_speed;
}


float FlightModel::getVerticalSpeed() const
{
	return m_verticalSpeed;
}
