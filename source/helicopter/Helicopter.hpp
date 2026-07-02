/*
 * Helicopter.hpp
 * 
 * 02-07-2026 by madpl
 */
#pragma once


class TerrainData;


class Helicopter
{
public:
	Helicopter();
	
	void setPosition(float x, float y, float z);
	void setYawDegrees(float yawDegrees);
	
	void update(float dt, const TerrainData& terrain);
	
	float getX() const;
	float getY() const;
	float getZ() const;
	
	float getYawDegrees() const;
	float getSpeed() const;
	float getAltitudeAboveGround() const;
	
private:
	void handleInput(float dt);
	void updateMovement(float dt);
	void updateTerrainRelation(const TerrainData& terrain);
	
private:
	float m_x;
	float m_y;
	float m_z;
	
	float m_yawDegrees;
	
	float m_speed;
	float m_verticalSpeed;
	
	float m_altitudeAboveGround;
	
	float m_maxForwardSpeed;
	float m_acceleration;
	float m_deceleration;
	float m_turnSpeed;
	float m_climbSpeed;
};
