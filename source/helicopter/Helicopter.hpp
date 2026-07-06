/*
 * Helicopter.hpp
 *
 * 02-07-2026 by madpl
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
	
	void setNetworkInputState(const HelicopterInputState& inputState);
	void clearNetworkInputOverride();
	
	void update(float dt, const TerrainData& terrain);
	
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
	
	FlightModel m_flightModel;
	HelicopterInput m_input;
	
	HelicopterInputState m_networkInputState;
	bool m_hasNetworkInputOverride;
};
