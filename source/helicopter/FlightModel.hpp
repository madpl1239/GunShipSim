/*
 * FlightModel.hpp
 */

#pragma once

#include <helicopter/HelicopterInput.hpp>

class FlightModel
{
public:
	FlightModel();
	
	void setYawDegrees(float yawDegrees);
	void update(float dt, const HelicopterInputState &inputState);
	
	float getYawDegrees() const;
	float getPitchDegrees() const;
	float getRollDegrees() const;
	float getSpeed() const;
	float getVerticalSpeed() const;
	
private:
	float m_yawDegrees;
	float m_pitchDegrees;
	float m_rollDegrees;
	
	float m_speed;
	float m_verticalSpeed;
	
	float m_maxForwardSpeed;
	float m_acceleration;
	float m_deceleration;
	float m_turnSpeed;
	float m_climbSpeed;
	
	float m_targetPitchDegrees;
	float m_targetRollDegrees;
};
