/*
 * HelicopterRenderer.hpp
 *
 * 08-07-2026 by madpl
 */
#pragma once

#include <cstdint>
#include <glm/glm.hpp>


class HelicopterRenderer
{
public:
	HelicopterRenderer();
	~HelicopterRenderer();
	
	bool create();
	void destroy();
	
	void renderBoxModel(const glm::mat4& viewProjectionMatrix,
						float x, float y, float z,
						float yawDegrees, float pitchDegrees,
						float rollDegrees, const glm::vec3& color) const;
					 
private:
	std::uint32_t m_vao;
	std::uint32_t m_vbo;
	std::uint32_t m_shaderProgram;
	std::int32_t m_uModel;
	std::int32_t m_uViewProjection;
	std::int32_t m_uColor;
	bool m_created;
};
