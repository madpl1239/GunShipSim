/*
 * TerrainRenderer.hpp
 * 
 * 02-07-2026 by madpl
 */
#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

class TerrainData;


class TerrainRenderer
{
public:
	TerrainRenderer();
	~TerrainRenderer();
	
	bool create(const TerrainData& terrain);
	void destroy();
	
	void render(const glm::mat4& mvpMatrix, const glm::vec3& cameraPosition);
	
	bool isValid() const;
	
private:
	bool buildMesh(const TerrainData& terrain);
	bool buildShaders();
	
	std::string readTextFile(const std::string& path) const;
	
	GLuint compileShader(GLenum type, const std::string& source);
	GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader);
	
	float heightToColorFactor(float h) const;
	
private:
	GLuint m_vao;
	GLuint m_vbo;
	GLuint m_ebo;
	GLuint m_shaderProgram;
	
	int m_width;
	int m_height;
	bool m_valid;
	
	std::vector<float> m_vertices;
	std::vector<unsigned int> m_indices;
};
