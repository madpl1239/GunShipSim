/*
 * Camera.cpp
 * 
 * Cocpit Camera - implementation.
 * 
 * 02-07-2026 by madpl
 */
#include <cmath>
#include <cstring>
#include <camera/Camera.hpp>


Camera::Camera():
	m_posX(0.0f),
	m_posY(2000.0f),
	m_posZ(2000.0f),
	m_targetX(0.0f),
	m_targetY(0.0f),
	m_targetZ(0.0f),
	m_fovDegrees(60.0f),
	m_aspectRatio(4.0f / 3.0f),
	m_nearPlane(1.0f),
	m_farPlane(50000.0f)
{
	std::memset(m_view, 0, sizeof(m_view));
	std::memset(m_projection, 0, sizeof(m_projection));
	std::memset(m_viewProjection, 0, sizeof(m_viewProjection));
	
	updateMatrices();
}


void Camera::setPerspective(float fovDegrees, float aspectRatio,
							float nearPlane, float farPlane)
{
	m_fovDegrees = fovDegrees;
	m_aspectRatio = aspectRatio;
	m_nearPlane = nearPlane;
	m_farPlane = farPlane;
}


void Camera::setPosition(float x, float y, float z)
{
	m_posX = x;
	m_posY = y;
	m_posZ = z;
}


void Camera::setTarget(float x, float y, float z)
{
	m_targetX = x;
	m_targetY = y;
	m_targetZ = z;
}


void Camera::move(float dx, float dy, float dz)
{
	m_posX += dx;
	m_posY += dy;
	m_posZ += dz;
}


void Camera::moveTarget(float dx, float dy, float dz)
{
	m_targetX += dx;
	m_targetY += dy;
	m_targetZ += dz;
}


const float* Camera::getViewMatrix() const
{
	return m_view;
}


const float* Camera::getProjectionMatrix() const
{
	return m_projection;
}


const float* Camera::getViewProjectionMatrix() const
{
	return m_viewProjection;
}


void Camera::updateMatrices()
{
	buildLookAt(m_view, m_posX, m_posY, m_posZ,
				m_targetX, m_targetY, m_targetZ,
				0.0f, 1.0f, 0.0f);
	
	buildPerspective(m_projection, m_fovDegrees, m_aspectRatio,
					 m_nearPlane, m_farPlane);
	
	multiply4x4(m_viewProjection, m_projection, m_view);
}


void Camera::buildPerspective(float* outMatrix, float fovDegrees, float aspectRatio,
							  float nearPlane, float farPlane)
{
	std::memset(outMatrix, 0, sizeof(float) * 16);
	
	float fovRadians = fovDegrees * 3.1415926535f / 180.0f;
	float tanHalfFov = std::tan(fovRadians * 0.5f);
	
	outMatrix[0] = 1.0f / (aspectRatio * tanHalfFov);
	outMatrix[5] = 1.0f / tanHalfFov;
	outMatrix[10] = -(farPlane + nearPlane) / (farPlane - nearPlane);
	outMatrix[11] = -1.0f;
	outMatrix[14] = -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane);
}


void Camera::buildLookAt(float* outMatrix, float eyeX, float eyeY, float eyeZ,
						 float targetX, float targetY, float targetZ,
						 float upX, float upY, float upZ)
{
	float fx = targetX - eyeX;
	float fy = targetY - eyeY;
	float fz = targetZ - eyeZ;
	normalize3(fx, fy, fz);
	
	float sx, sy, sz;
	cross3(fx, fy, fz, upX, upY, upZ, sx, sy, sz);
	normalize3(sx, sy, sz);
	
	float ux, uy, uz;
	cross3(sx, sy, sz, fx, fy, fz, ux, uy, uz);
	
	std::memset(outMatrix, 0, sizeof(float) * 16);
	
	outMatrix[0] = sx;
	outMatrix[4] = sy;
	outMatrix[8] = sz;
	
	outMatrix[1] = ux;
	outMatrix[5] = uy;
	outMatrix[9] = uz;
	
	outMatrix[2] = -fx;
	outMatrix[6] = -fy;
	outMatrix[10] = -fz;
	
	outMatrix[12] = -dot3(sx, sy, sz, eyeX, eyeY, eyeZ);
	outMatrix[13] = -dot3(ux, uy, uz, eyeX, eyeY, eyeZ);
	outMatrix[14] = dot3(fx, fy, fz, eyeX, eyeY, eyeZ);
	outMatrix[15] = 1.0f;
}


void Camera::multiply4x4(float* outMatrix, const float* a, const float* b)
{
	float result[16];
	
	for(int row = 0; row < 4; ++row)
	{
		for(int col = 0; col < 4; ++col)
		{
			result[col + row * 4] =
			a[0 + row * 4] * b[col + 0 * 4] +
			a[1 + row * 4] * b[col + 1 * 4] +
			a[2 + row * 4] * b[col + 2 * 4] +
			a[3 + row * 4] * b[col + 3 * 4];
		}
	}
	
	std::memcpy(outMatrix, result, sizeof(result));
}


void Camera::normalize3(float& x, float& y, float& z)
{
	float len = std::sqrt(x * x + y * y + z * z);
	
	if(len > 0.000001f)
	{
		x /= len;
		y /= len;
		z /= len;
	}
}


void Camera::cross3(float ax, float ay, float az,
					float bx, float by, float bz,
					float& outX, float& outY, float& outZ)
{
	outX = ay * bz - az * by;
	outY = az * bx - ax * bz;
	outZ = ax * by - ay * bx;
}


float Camera::dot3(float ax, float ay, float az,
				   float bx, float by, float bz)
{
	return ax * bx + ay * by + az * bz;
}
