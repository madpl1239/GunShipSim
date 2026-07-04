/*
 * main.cpp
 *
 * GunShip Simulator.
 *
 * 02-07-2026 by madpl
 */
#include <iostream>
#include <cmath>
#include <sstream>
#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/System/Clock.hpp>
#include <terrain/HGTLoader.hpp>
#include <terrain/TerrainData.hpp>
#include <terrain/TerrainRenderer.hpp>
#include <camera/Camera.hpp>
#include <helicopter/Helicopter.hpp>
#include <hud/HUD.hpp>


int main(void)
{
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 6;
	settings.majorVersion = 3;
	settings.minorVersion = 3;
	settings.attributeFlags = 0;
	
	sf::RenderWindow window(sf::VideoMode(1280, 720), "GunSim v 0.1 by madpl 2026",
							sf::Style::Titlebar | sf::Style::Close, settings);
	
	window.setVerticalSyncEnabled(false);
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
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor(0.60f, 0.75f, 0.95f, 1.0f);
	
	HUD hud;
	if(not hud.initialize("fonts/DejaVuSans.ttf"))
	{
		std::cerr << "Failed to load HUD font\n";
		
		return -1;
	}
	
	HGTLoader loader;
	HGTLoader::Data rawData;
	
	if(not loader.load("res/terrain/N34E062.hgt", rawData))
	{
		std::cerr << "Failed to load HGT file\n";
		
		return -1;
	}
	
	std::cout << "rawData.width = " << rawData.width << "\n";
	std::cout << "rawData.height = " << rawData.height << "\n";
	
	TerrainData terrain;
	if(not terrain.buildFromHGT(rawData.samples, rawData.width, rawData.height,
								34.0f, 62.0f, 34.5f, 62.5f, 12000.0f, 256))
	{
		std::cerr << "Failed to build terrain data\n";
		
		return -1;
	}
	
	std::cout << "terrain width = " << terrain.getWorldSizeX() << "\n";
	std::cout << "terrain height = " << terrain.getWorldSizeZ() << "\n";
	std::cout << "spacing = "
	<< terrain.getWorldSizeX() / float(terrain.getWidth() - 1)
	<< "\n";
	
	TerrainRenderer renderer;
	if(not renderer.create(terrain))
	{
		std::cerr << "Failed to create terrain renderer\n";
		
		return -1;
	}
	
	Helicopter helicopter;
	helicopter.setPosition(0.0f, terrain.getHeightAtWorldPosition(0.0f, 0.0f) + 5.0f, 0.0f);
	helicopter.setYawDegrees(0.0f);
	
	Camera camera;
	camera.setPerspective(75.0f, 1280.0f / 720.0f, 0.3f, 100000.0f);
	camera.updateMatrices();
	
	sf::Clock frameClock;
	float previousAltitude = helicopter.getY();
	
	bool running = true;
	
	while(running)
	{
		float dt = frameClock.restart().asSeconds();
		
		if(dt > 0.1f)
			dt = 0.1f;
		
		sf::Event event;
		while(window.pollEvent(event))
		{
			if(event.type == sf::Event::Closed)
				running = false;
			
			else if(event.type == sf::Event::KeyPressed)
			{
				if(event.key.code == sf::Keyboard::Escape)
					running = false;
			}
			
			else if(event.type == sf::Event::Resized)
			{
				glViewport(0, 0, static_cast<GLsizei>(event.size.width),
						   static_cast<GLsizei>(event.size.height));
				
				float aspect = static_cast<float>(event.size.width) / static_cast<float>(event.size.height);
				
				camera.setPerspective(75.0f, aspect, 0.3f, 100000.0f);
			}
		}
		
		helicopter.update(dt, terrain);
		
		float verticalSpeed = 0.0f;
		if(dt > 0.0001f)
			verticalSpeed = (helicopter.getY() - previousAltitude) / dt;
		
		previousAltitude = helicopter.getY();
		
		const float pi = 3.1415926535f;
		
		float yawRadians = helicopter.getYawDegrees() * pi / 180.0f;
		float pitchRadians = helicopter.getPitchDegrees() * pi / 180.0f;
		
		float forwardX = std::sin(yawRadians) * std::cos(pitchRadians);
		float forwardY = std::sin(pitchRadians);
		float forwardZ = -std::cos(yawRadians) * std::cos(pitchRadians);
		
		float cockpitForwardOffset = 1.2f;
		float cockpitUpOffset = 0.6f;
		float lookDistance = 250.0f;
		
		float camX = helicopter.getX() + forwardX * cockpitForwardOffset;
		float camY = helicopter.getY() + cockpitUpOffset;
		float camZ = helicopter.getZ() + forwardZ * cockpitForwardOffset;
		
		float targetX = camX + forwardX * lookDistance;
		float targetY = camY + forwardY * lookDistance;
		float targetZ = camZ + forwardZ * lookDistance;
		
		camera.setPosition(camX, camY, camZ);
		camera.setTarget(targetX, targetY, targetZ);
		camera.setRollDegrees(-helicopter.getRollDegrees());
		camera.updateMatrices();
		
		hud.setHeadingDegrees(helicopter.getYawDegrees());
		hud.setAltitudeMeters(helicopter.getY());
		hud.setAltitudeAboveGroundMeters(helicopter.getAltitudeAboveGround());
		hud.setSpeedMetersPerSecond(helicopter.getSpeed());
		hud.setVerticalSpeedMetersPerSecond(verticalSpeed);
		
		/*
		std::ostringstream hud;
		hud
		<< "GunShipSim"
		<< " | SPD " << static_cast<int>(helicopter.getSpeed()) << " m/s"
		<< " | ALT " << static_cast<int>(helicopter.getY()) << " m"
		<< " | AGL " << static_cast<int>(helicopter.getAltitudeAboveGround()) << " m"
		<< " | VS " << static_cast<int>(helicopter.getVerticalSpeed()) << " m/s"
		<< " | HDG " << static_cast<int>(helicopter.getYawDegrees()) << " deg"
		<< " | PITCH " << static_cast<int>(helicopter.getPitchDegrees()) << " deg"
		<< " | ROLL " << static_cast<int>(helicopter.getRollDegrees()) << " deg";
		window.setTitle(hud.str());
		
		debugTimer += dt;
		if(debugTimer >= 20.0f)
		{
			debugTimer = 0.0f;
			
			float terrainHeight = terrain.getHeightAtWorldPosition(
				helicopter.getX(),
																   helicopter.getZ());
			
			std::cout << "Heli pos = ("
			<< helicopter.getX() << ", "
			<< helicopter.getY() << ", "
			<< helicopter.getZ() << ") "
			<< "terrainY = " << terrainHeight << " "
			<< "AGL = " << helicopter.getAltitudeAboveGround() << " "
			<< "speed = " << helicopter.getSpeed() << " "
			<< "vs = " << helicopter.getVerticalSpeed() << " "
			<< "yaw = " << helicopter.getYawDegrees() << " "
			<< "pitch = " << helicopter.getPitchDegrees() << " "
			<< "roll = " << helicopter.getRollDegrees()
			<< "\n";
		}
		*/
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		renderer.render(camera.getViewProjectionMatrix(), glm::vec3(camX, camY, camZ));
		
		// HUD
		window.pushGLStates();
		hud.draw(window);
		window.popGLStates();
		
		window.display();
	}
	
	renderer.destroy();
	window.close();
	
	return 0;
}
