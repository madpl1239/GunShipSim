/*
 * HUD.hpp
 * 
 * 04-07-2026 by madpl
 */
#pragma once

#include <SFML/Graphics.hpp>
#include <hud/AltimeterWidget.hpp>
#include <hud/CompassWidget.hpp>


class HUD
{
public:
	HUD();
	
	bool initialize(const char *fontPath);
	
	void setHeadingDegrees(float headingDegrees);
	void setAltitudeMeters(float altitudeMeters);
	void setAltitudeAboveGroundMeters(float altitudeAboveGroundMeters);
	void setSpeedMetersPerSecond(float speedMetersPerSecond);
	void setVerticalSpeedMetersPerSecond(float verticalSpeedMetersPerSecond);
	
	void draw(sf::RenderWindow &window);
	
private:
	sf::Font m_font;
	bool m_isInitialized;
	
	float m_headingDegrees;
	float m_altitudeMeters;
	float m_altitudeAboveGroundMeters;
	float m_speedMetersPerSecond;
	float m_verticalSpeedMetersPerSecond;
	
	CompassWidget m_compassWidget;
	AltimeterWidget m_altimeterWidget;
};
