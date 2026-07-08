/*
 * HelicopterRenderer.cpp
 *
 * 08-07-2026 by madpl
 */
#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <helicopter/HelicopterRenderer.hpp>


namespace
{
	std::uint32_t compileShader(GLenum type, const char* source)
	{
		const std::uint32_t shader = glCreateShader(type);
		glShaderSource(shader, 1, &source, nullptr);
		glCompileShader(shader);
		
		GLint success = GL_FALSE;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		
		if(success != GL_TRUE)
		{
			GLchar log[1024];
			glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
			std::cerr << "HelicopterRenderer shader compile failed: " << log << "\n";
			glDeleteShader(shader);
			
			return 0;
		}
		
		return shader;
	}
	
	std::uint32_t createProgram(const char* vertexSource, const char* fragmentSource)
	{
		const std::uint32_t vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
		if(vertexShader == 0)
			return 0;
		
		const std::uint32_t fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
		if(fragmentShader == 0)
		{
			glDeleteShader(vertexShader);
			
			return 0;
		}
		
		const std::uint32_t program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		glLinkProgram(program);
		
		GLint success = GL_FALSE;
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		
		if(success != GL_TRUE)
		{
			GLchar log[1024];
			glGetProgramInfoLog(program, sizeof(log), nullptr, log);
			std::cerr << "HelicopterRenderer program link failed: " << log << "\n";
			glDeleteProgram(program);
			
			return 0;
		}
		
		return program;
	}
}


HelicopterRenderer::HelicopterRenderer():
	m_vao(0),
	m_vbo(0),
	m_shaderProgram(0),
	m_uModel(-1),
	m_uViewProjection(-1),
	m_uColor(-1),
	m_created(false)
{
	// no-op
}


HelicopterRenderer::~HelicopterRenderer()
{
	destroy();
}


bool HelicopterRenderer::create()
{
	destroy();
	
	static const char* vertexShaderSource = R"(
	#version 330 core
	layout(location = 0) in vec3 aPosition;
	
	uniform mat4 uModel;
	uniform mat4 uViewProjection;
	
	void main()
	{
		gl_Position = uViewProjection * uModel * vec4(aPosition, 1.0);
	})";

	static const char* fragmentShaderSource = R"(
	#version 330 core
	
	uniform vec3 uColor;
	
	out vec4 fragColor;
	
	void main()
	{
		fragColor = vec4(uColor, 1.0);
	})";

	m_shaderProgram = createProgram(vertexShaderSource, fragmentShaderSource);
	if(m_shaderProgram == 0)
		return false;

	m_uModel = glGetUniformLocation(m_shaderProgram, "uModel");
	m_uViewProjection = glGetUniformLocation(m_shaderProgram, "uViewProjection");
	m_uColor = glGetUniformLocation(m_shaderProgram, "uColor");

	const float vertices[] =
	{
		-0.5f, -0.5f,  0.5f,   0.5f, -0.5f,  0.5f,   0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,   0.5f,  0.5f,  0.5f,  -0.5f,  0.5f,  0.5f,
		
		-0.5f, -0.5f, -0.5f,  -0.5f,  0.5f, -0.5f,   0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,   0.5f,  0.5f, -0.5f,   0.5f, -0.5f, -0.5f,
		
		-0.5f, -0.5f, -0.5f,  -0.5f, -0.5f,  0.5f,  -0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,  -0.5f,  0.5f,  0.5f,  -0.5f,  0.5f, -0.5f,
		
		0.5f, -0.5f, -0.5f,   0.5f,  0.5f, -0.5f,   0.5f,  0.5f,  0.5f,
		0.5f, -0.5f, -0.5f,   0.5f,  0.5f,  0.5f,   0.5f, -0.5f,  0.5f,
		
		-0.5f,  0.5f, -0.5f,  -0.5f,  0.5f,  0.5f,   0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,   0.5f,  0.5f,  0.5f,   0.5f,  0.5f, -0.5f,
		
		-0.5f, -0.5f, -0.5f,   0.5f, -0.5f, -0.5f,   0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,   0.5f, -0.5f,  0.5f,  -0.5f, -0.5f,  0.5f
	};

	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	m_created = true;
	
	return true;
}


void HelicopterRenderer::destroy()
{
	if(m_vbo != 0)
	{
		glDeleteBuffers(1, &m_vbo);
		m_vbo = 0;
	}
	
	if(m_vao != 0)
	{
		glDeleteVertexArrays(1, &m_vao);
		m_vao = 0;
	}
	
	if(m_shaderProgram != 0)
	{
		glDeleteProgram(m_shaderProgram);
		m_shaderProgram = 0;
	}
	
	m_uModel = -1;
	m_uViewProjection = -1;
	m_uColor = -1;
	m_created = false;
}


void HelicopterRenderer::renderBoxModel(const glm::mat4& viewProjectionMatrix,
										float x, float y, float z, float yawDegrees,
										float pitchDegrees, float rollDegrees, const glm::vec3& color) const
{
	if(not m_created)
		return;

	glm::mat4 model(1.0f);
	model = glm::translate(model, glm::vec3(x, y, z));
	model = glm::rotate(model, glm::radians(yawDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(pitchDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-rollDegrees), glm::vec3(0.0f, 0.0f, 1.0f));

	const glm::mat4 fuselageModel = glm::scale(model, glm::vec3(1.4f, 0.5f, 3.8f));

	const glm::mat4 tailModel = glm::translate(model, glm::vec3(0.0f, 0.05f, 2.8f)) *
									glm::scale(glm::mat4(1.0f), glm::vec3(0.22f, 0.22f, 2.8f));

	const glm::vec3 tailColor = color * 0.75f;

	glUseProgram(m_shaderProgram);
	glUniformMatrix4fv(m_uViewProjection, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));

	glBindVertexArray(m_vao);

	glUniform3fv(m_uColor, 1, glm::value_ptr(color));
	glUniformMatrix4fv(m_uModel, 1, GL_FALSE, glm::value_ptr(fuselageModel));
	glDrawArrays(GL_TRIANGLES, 0, 36);
	
	glUniform3fv(m_uColor, 1, glm::value_ptr(tailColor));
	glUniformMatrix4fv(m_uModel, 1, GL_FALSE, glm::value_ptr(tailModel));
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glBindVertexArray(0);
	glUseProgram(0);
}
