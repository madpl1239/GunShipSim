/*
 * TerrainRenderer.cpp
 * 
 * 02-07-2026 by madpl
 */
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>
#include <terrain/TerrainData.hpp>
#include <terrain/TerrainRenderer.hpp>


TerrainRenderer::TerrainRenderer():
	m_vao(0),
	m_vbo(0),
	m_ebo(0),
	m_shaderProgram(0),
	m_width(0),
	m_height(0),
	m_valid(false)
{
	// empty
}


TerrainRenderer::~TerrainRenderer()
{
	destroy();
}


bool TerrainRenderer::create(const TerrainData& terrain)
{
	destroy();
	
	if(not terrain.isValid())
		return false;
	
	if(not buildMesh(terrain))
		return false;
	
	if(not buildShaders())
		return false;
	
	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);
	glGenBuffers(1, &m_ebo);
	
	if(m_vao == 0 or m_vbo == 0 or m_ebo == 0)
	{
		destroy();
		
		return false;
	}
	
	glBindVertexArray(m_vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_vertices.size() * sizeof(float)),
				 m_vertices.data(), GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_indices.size() * sizeof(unsigned int)),
				 m_indices.data(), GL_STATIC_DRAW);
	
	const GLsizei stride = 4 * sizeof(float);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));
	
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float)));
	
	glBindVertexArray(0);
	
	m_valid = true;
	
	std::cout << "vertices: " << m_vertices.size() << "\n";
	std::cout << "indices: " << m_indices.size() << "\n";
	
	return true;
}


void TerrainRenderer::destroy()
{
	if(m_shaderProgram != 0)
	{
		glDeleteProgram(m_shaderProgram);
		m_shaderProgram = 0;
	}
	
	if(m_vbo != 0)
	{
		glDeleteBuffers(1, &m_vbo);
		m_vbo = 0;
	}
	
	if(m_ebo != 0)
	{
		glDeleteBuffers(1, &m_ebo);
		m_ebo = 0;
	}
	
	if(m_vao != 0)
	{
		glDeleteVertexArrays(1, &m_vao);
		m_vao = 0;
	}
	
	m_vertices.clear();
	m_indices.clear();
	
	m_width = 0;
	m_height = 0;
	m_valid = false;
}


void TerrainRenderer::render(const glm::mat4& mvpMatrix)
{
	if(not m_valid or m_vao == 0 or m_shaderProgram == 0)
		return;
	
	glUseProgram(m_shaderProgram);
	
	GLint mvpLocation = glGetUniformLocation(m_shaderProgram, "uMVP");
	if(mvpLocation >= 0)
		glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
	
	glBindVertexArray(m_vao);
	
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()),
				   GL_UNSIGNED_INT, reinterpret_cast<void*>(0));
	
	glBindVertexArray(0);
	
	glUseProgram(0);
}


bool TerrainRenderer::isValid() const
{
	return m_valid;
}


bool TerrainRenderer::buildMesh(const TerrainData& terrain)
{
	m_width = terrain.getWidth();
	m_height = terrain.getHeight();
	
	if(m_width <= 1 or m_height <= 1)
		return false;
	
	m_vertices.clear();
	m_indices.clear();
	
	m_vertices.reserve(static_cast<std::size_t>(m_width * m_height * 4));
	
	const float worldSizeX = terrain.getWorldSizeX();
	const float worldSizeZ = terrain.getWorldSizeZ();
	
	const float centerX = worldSizeX * 0.5f;
	const float centerZ = worldSizeZ * 0.5f;
	
	const float heightScale = 1.0f;
	const float baseHeight = terrain.getMinHeight();
	
	for(int z = 0; z < m_height; ++z)
	{
		for(int x = 0; x < m_width; ++x)
		{
			float fx = static_cast<float>(x) / static_cast<float>(m_width - 1);
			float fz = static_cast<float>(z) / static_cast<float>(m_height - 1);
			
			float worldX = fx * worldSizeX - centerX;
			float worldZ = fz * worldSizeZ - centerZ;
			
			float rawHeight = terrain.getHeightAtGrid(x, z);
			float worldY = (rawHeight - baseHeight) * heightScale;
			
			float colorFactor = heightToColorFactor(rawHeight);
			
			m_vertices.push_back(worldX);
			m_vertices.push_back(worldY);
			m_vertices.push_back(worldZ);
			m_vertices.push_back(colorFactor);
		}
	}
	
	for(int z = 0; z < m_height - 1; ++z)
	{
		for(int x = 0; x < m_width - 1; ++x)
		{
			unsigned int i0 = static_cast<unsigned int>(z * m_width + x);
			unsigned int i1 = static_cast<unsigned int>(z * m_width + x + 1);
			unsigned int i2 = static_cast<unsigned int>((z + 1) * m_width + x);
			unsigned int i3 = static_cast<unsigned int>((z + 1) * m_width + x + 1);
			
			m_indices.push_back(i0);
			m_indices.push_back(i2);
			m_indices.push_back(i1);
			
			m_indices.push_back(i1);
			m_indices.push_back(i2);
			m_indices.push_back(i3);
		}
	}
	
	return true;
}


bool TerrainRenderer::buildShaders()
{
	const std::string vertexSource = R"(
		#version 330 core
		layout(location = 0) in vec3 aPos;
		layout(location = 1) in float aHeightFactor;
		
		uniform mat4 uMVP;
		
		out float vHeightFactor;
		
		void main()
		{
			vHeightFactor = aHeightFactor;
			gl_Position = uMVP * vec4(aPos, 1.0);
		}
	)";

	const std::string fragmentSource = R"(
		#version 330 core
		in float vHeightFactor;
		out vec4 FragColor;
		
		void main()
		{
			vec3 lowColor = vec3(0.72, 0.62, 0.38);
			vec3 highColor = vec3(0.40, 0.36, 0.24);
			vec3 color = mix(lowColor, highColor, clamp(vHeightFactor, 0.0, 1.0));
			FragColor = vec4(color, 1.0);
		}
	)";

	GLuint vs = compileShader(GL_VERTEX_SHADER, vertexSource);
	if(vs == 0)
	{
		return false;
	}

	GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
	if(fs == 0)
	{
		glDeleteShader(vs);
		return false;
	}

	GLuint program = linkProgram(vs, fs);

	glDeleteShader(vs);
	glDeleteShader(fs);

	if(program == 0)
		return false;

	m_shaderProgram = program;

	return true;
}


GLuint TerrainRenderer::compileShader(GLenum type, const std::string& source)
{
	GLuint shader = glCreateShader(type);
	if(shader == 0)
		return 0;
	
	const char* src = source.c_str();
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);
	
	GLint status = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if(status != GL_TRUE)
	{
		GLint logLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
		
		if(logLength > 1)
		{
			std::string log(static_cast<std::size_t>(logLength), '\0');
			glGetShaderInfoLog(shader, logLength, nullptr, log.data());
			std::cerr << log << std::endl;
		}
		
		glDeleteShader(shader);
		
		return 0;
	}
	
	return shader;
}


GLuint TerrainRenderer::linkProgram(GLuint vertexShader, GLuint fragmentShader)
{
	GLuint program = glCreateProgram();
	if(program == 0)
		return 0;
	
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	
	GLint status = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if(status != GL_TRUE)
	{
		GLint logLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		
		if(logLength > 1)
		{
			std::string log(static_cast<std::size_t>(logLength), '\0');
			glGetProgramInfoLog(program, logLength, nullptr, log.data());
			std::cerr << log << std::endl;
		}
		
		glDeleteProgram(program);
		
		return 0;
	}
	
	return program;
}


float TerrainRenderer::heightToColorFactor(float h) const
{
	float normalized = (h + 500.0f) / 1500.0f;
	
	return std::clamp(normalized, 0.0f, 1.0f);
}
