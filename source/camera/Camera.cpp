/*
 * Camera.cpp
 *
 * 04-07-2027 by madpl
 */
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <camera/Camera.hpp>


Camera::Camera():
	m_position(0.0f, 6000.0f, 12000.0f),
	m_target(0.0f, 0.0f, 0.0f),
	m_up(0.0f, 1.0f, 0.0f),
	m_rollDegrees(0.0f),
	m_fovDegrees(60.0f),
	m_aspectRatio(4.0f / 3.0f),
	m_nearPlane(1.0f),
	m_farPlane(50000.0f),
	m_view(1.0f),
	m_projection(1.0f),
	m_viewProjection(1.0f)
{
	updateMatrices();
}


void Camera::setPerspective(float fovDegrees, float aspectRatio, float nearPlane, float farPlane)
{
	m_fovDegrees = fovDegrees;
	m_aspectRatio = aspectRatio;
	m_nearPlane = nearPlane;
	m_farPlane = farPlane;
}


void Camera::setPosition(float x, float y, float z)
{
	m_position = glm::vec3(x, y, z);
}


void Camera::setTarget(float x, float y, float z)
{
	m_target = glm::vec3(x, y, z);
}


void Camera::setRollDegrees(float rollDegrees)
{
	m_rollDegrees = rollDegrees;
}


void Camera::move(float dx, float dy, float dz)
{
	m_position += glm::vec3(dx, dy, dz);
}


void Camera::moveTarget(float dx, float dy, float dz)
{
	m_target += glm::vec3(dx, dy, dz);
}


void Camera::updateMatrices()
{
	glm::vec3 forward = glm::normalize(m_target - m_position);
	
	glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
	glm::vec3 right = glm::normalize(glm::cross(forward, worldUp));
	glm::vec3 baseUp = glm::normalize(glm::cross(right, forward));
	
	float rollRadians = glm::radians(m_rollDegrees);
	
	m_up = glm::normalize(baseUp * std::cos(rollRadians) + right * std::sin(rollRadians));
	
	m_view = glm::lookAt(m_position, m_target, m_up);
	m_projection = glm::perspective(glm::radians(m_fovDegrees), m_aspectRatio,
									m_nearPlane, m_farPlane);
	
	m_viewProjection = m_projection * m_view;
}


const glm::vec3& Camera::getPosition() const
{
	return m_position;
}


const glm::vec3& Camera::getTarget() const
{
	return m_target;
}


const glm::mat4& Camera::getViewMatrix() const
{
	return m_view;
}


const glm::mat4& Camera::getProjectionMatrix() const
{
	return m_projection;
}


const glm::mat4& Camera::getViewProjectionMatrix() const
{
	return m_viewProjection;
}
