/*
* Helicopter.hpp
*
* 07-07-2026 by madpl
*/
#pragma once

#include <terrain/TerrainData.hpp>
#include <helicopter/FlightModel.hpp>
#include <helicopter/HelicopterInput.hpp>


class Helicopter
{
public:
	Helicopter();

	void setPosition(float x, float y, float z);
	void setYawDegrees(float yawDegrees);

	void update(float dt, const TerrainData& terrain);
	void update(float dt, const TerrainData& terrain, const HelicopterInputState& inputState);

	void applyAuthoritativeState(float x, float y, float z,
								 float yawDegrees, float pitchDegrees, float rollDegrees,
								 float speedMetersPerSecond, float verticalSpeedMetersPerSecond,
								 float altitudeAboveGroundMeters);

	float getX() const;
	float getY() const;
	float getZ() const;

	float getYawDegrees() const;
	float getPitchDegrees() const;
	float getRollDegrees() const;

	float getSpeed() const;
	float getVerticalSpeed() const;
	float getAltitudeAboveGround() const;

private:
	void updateMovement(float dt);
	void updateTerrainRelation(const TerrainData& terrain);

	float m_x;
	float m_y;
	float m_z;
	float m_altitudeAboveGround;

	float m_authoritativeYawDegrees;
	float m_authoritativePitchDegrees;
	float m_authoritativeRollDegrees;
	float m_authoritativeSpeed;
	float m_authoritativeVerticalSpeed;
	bool m_hasAuthoritativeOverride;

	FlightModel m_flightModel;
	HelicopterInput m_input;
};
