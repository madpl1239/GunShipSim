/*
 * Camera.hpp
 * 
 * Cocpit Camera for GunShip Simulator.
 * 
 * 02-07-2026 by madpl
 */
#pragma once

#include <cstring>


class Camera
{
public:
	Camera();
	
	void setPerspective(float fovDegrees, float aspectRatio, float nearPlane, float farPlane);
	void setPosition(float x, float y, float z);
	void setTarget(float x, float y, float z);
	
	void move(float dx, float dy, float dz);
	void moveTarget(float dx, float dy, float dz);
	
	const float* getViewMatrix() const;
	const float* getProjectionMatrix() const;
	const float* getViewProjectionMatrix() const;
	
	void updateMatrices();
	
private:
	void buildPerspective(float* outMatrix, float fovDegrees, float aspectRatio,
						  float nearPlane, float farPlane);
	
	void buildLookAt(float* outMatrix, float eyeX, float eyeY, float eyeZ,
						float targetX, float targetY, float targetZ,
						float upX, float upY, float upZ);
	
	void multiply4x4(float* outMatrix, const float* a, const float* b);
	
	void normalize3(float& x, float& y, float& z);
	
	void cross3(float ax, float ay, float az,
				float bx, float by, float bz,
				float& outX, float& outY, float& outZ);
	
	float dot3(float ax, float ay, float az,
			   float bx, float by, float bz);
	
private:
	float m_posX;
	float m_posY;
	float m_posZ;
	
	float m_targetX;
	float m_targetY;
	float m_targetZ;
	
	float m_fovDegrees;
	float m_aspectRatio;
	float m_nearPlane;
	float m_farPlane;
	
	float m_view[16];
	float m_projection[16];
	float m_viewProjection[16];
};
