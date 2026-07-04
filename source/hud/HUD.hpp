#pragma once

#include <SFML/Graphics.hpp>
#include <hud/AltimeterWidget.hpp>
#include <hud/CompassWidget.hpp>
#include <hud/SpeedWidget.hpp>
#include <hud/AGLWidget.hpp>
#include <hud/VerticalSpeedWidget.hpp>


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
	SpeedWidget m_speedWidget;
	AGLWidget m_aglWidget;
	VerticalSpeedWidget m_verticalSpeedWidget;
};
