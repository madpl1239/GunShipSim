/*
 * main.cpp
 * 
 * GunShip Simulator.
 * 
 * 02-07-2026 by madpl
 */
#include <iostream>
#include <GL/glew.h>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <terrain/HGTLoader.hpp>
#include <terrain/TerrainData.hpp>
#include <terrain/TerrainRenderer.hpp>
#include <camera/Camera.hpp>


int main(void)
{
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 4;
	settings.majorVersion = 3;
	settings.minorVersion = 3;
	settings.attributeFlags = sf::ContextSettings::Core;
	
	sf::Window window(sf::VideoMode(1280, 720), "GunShipSim - Terrain Test",
				   sf::Style::Titlebar | sf::Style::Close, settings);
	
	window.setVerticalSyncEnabled(true);
	window.setActive(true);
	
	glewExperimental = GL_TRUE;
	GLenum glewStatus = glewInit();
	if(glewStatus != GLEW_OK)
	{
		std::cerr << "glewInit failed\n";
		
		return -1;
	}
	
	glGetError();
	
	glViewport(0, 0, 1280, 720);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glClearColor(0.60f, 0.75f, 0.95f, 1.0f);
	
	HGTLoader loader;
	HGTLoader::Data rawData;
	
	if(not loader.load("res/terrain/N34E062.hgt", rawData))
	{
		std::cerr << "Failed to load HGT file\n";
		
		return -1;
	}
	
	TerrainData terrain;
	if(not terrain.buildFromHGT(rawData.samples, rawData.width, rawData.height, 12000.0f, 256))
	{
		std::cerr << "Failed to build terrain data\n";
		
		return -1;
	}
	
	TerrainRenderer renderer;
	if(not renderer.create(terrain))
	{
		std::cerr << "Failed to create terrain renderer\n";
		
		return -1;
	}
	
	Camera camera;
	camera.setPerspective(60.0f, 1280.0f / 720.0f, 1.0f, 50000.0f);
	
	camera.setPosition(0.0f, 6000.0f, 12000.0f);
	camera.setTarget(0.0f, 0.0f, 0.0f);
	
	// camera.setPosition(6000.0f, 4000.0f, 14000.0f);
	// camera.setTarget(6000.0f, 0.0f, 6000.0f);
	camera.updateMatrices();
	
	bool running = true;
	
	while(running)
	{
		sf::Event event;
		while(window.pollEvent(event))
		{
			if(event.type == sf::Event::Closed)
			{
				running = false;
			}
			
			if(event.type == sf::Event::KeyPressed)
			{
				if(event.key.code == sf::Keyboard::Escape)
					running = false;
				
				else if(event.key.code == sf::Keyboard::Up)
				{
					camera.move(0.0f, 0.0f, -500.0f);
					camera.moveTarget(0.0f, 0.0f, -500.0f);
				}
				
				else if(event.key.code == sf::Keyboard::Down)
				{
					camera.move(0.0f, 0.0f, 500.0f);
					camera.moveTarget(0.0f, 0.0f, 500.0f);
				}
				
				else if(event.key.code == sf::Keyboard::Left)
				{
					camera.move(-500.0f, 0.0f, 0.0f);
					camera.moveTarget(-500.0f, 0.0f, 0.0f);
				}
				
				else if(event.key.code == sf::Keyboard::Right)
				{
					camera.move(500.0f, 0.0f, 0.0f);
					camera.moveTarget(500.0f, 0.0f, 0.0f);
				}
				
				else if(event.key.code == sf::Keyboard::PageUp)
					camera.move(0.0f, 500.0f, 0.0f);
				
				else if(event.key.code == sf::Keyboard::PageDown)
					camera.move(0.0f, -500.0f, 0.0f);
				
				camera.updateMatrices();
			}
			
			if(event.type == sf::Event::Resized)
			{
				glViewport(0, 0, static_cast<GLsizei>(event.size.width),
						   static_cast<GLsizei>(event.size.height));
				
				float aspect = static_cast<float>(event.size.width) / static_cast<float>(event.size.height);
				camera.setPerspective(60.0f, aspect, 1.0f, 50000.0f);
				camera.updateMatrices();
			}
		}
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		renderer.render(camera.getViewProjectionMatrix());
		
		window.display();
	}
	
	renderer.destroy();
	window.close();
	
	return 0;
}
