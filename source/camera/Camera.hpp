/*
 * Camera.hpp
 * 
 * 02-07-2027 by madpl
 */
#pragma once

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>


class Camera
{
public:
	Camera();
	
	void setPerspective(float fovDegrees, float aspectRatio, float nearPlane, float farPlane);
	void setPosition(float x, float y, float z);
	void setTarget(float x, float y, float z);
	
	void move(float dx, float dy, float dz);
	void moveTarget(float dx, float dy, float dz);
	
	void updateMatrices();
	
	const glm::vec3& getPosition() const;
	const glm::vec3& getTarget() const;
	
	const glm::mat4& getViewMatrix() const;
	const glm::mat4& getProjectionMatrix() const;
	const glm::mat4& getViewProjectionMatrix() const;
	
private:
	glm::vec3 m_position;
	glm::vec3 m_target;
	glm::vec3 m_up;
	
	float m_fovDegrees;
	float m_aspectRatio;
	float m_nearPlane;
	float m_farPlane;
	
	glm::mat4 m_view;
	glm::mat4 m_projection;
	glm::mat4 m_viewProjection;
};
