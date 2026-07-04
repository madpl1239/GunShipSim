/*
 * HUD.cpp
 */

#include <hud/HUD.hpp>

HUD::HUD()
: m_isInitialized(false),
m_headingDegrees(0.0f),
m_altitudeMeters(0.0f),
m_altitudeAboveGroundMeters(0.0f),
m_speedMetersPerSecond(0.0f),
m_verticalSpeedMetersPerSecond(0.0f)
{
}

bool HUD::initialize(const char *fontPath)
{
	if(not m_font.loadFromFile(fontPath))
	{
		m_isInitialized = false;
		
		return false;
	}
	
	m_compassWidget.setFont(&m_font);
	m_altimeterWidget.setFont(&m_font);
	m_speedWidget.setFont(&m_font);
	m_aglWidget.setFont(&m_font);
	m_verticalSpeedWidget.setFont(&m_font);
	
	m_isInitialized = true;
	
	return true;
}

void HUD::setHeadingDegrees(float headingDegrees)
{
	m_headingDegrees = headingDegrees;
}

void HUD::setAltitudeMeters(float altitudeMeters)
{
	m_altitudeMeters = altitudeMeters;
}

void HUD::setAltitudeAboveGroundMeters(float altitudeAboveGroundMeters)
{
	m_altitudeAboveGroundMeters = altitudeAboveGroundMeters;
}

void HUD::setSpeedMetersPerSecond(float speedMetersPerSecond)
{
	m_speedMetersPerSecond = speedMetersPerSecond;
}

void HUD::setVerticalSpeedMetersPerSecond(float verticalSpeedMetersPerSecond)
{
	m_verticalSpeedMetersPerSecond = verticalSpeedMetersPerSecond;
}

void HUD::draw(sf::RenderWindow &window)
{
	if(not m_isInitialized)
	{
		return;
	}
	
	sf::Vector2u windowSize = window.getSize();
	
	float hudHeight = 202.5f;
	float gaugeCenterY = static_cast<float>(windowSize.y) - hudHeight * 0.5f;
	
	sf::RectangleShape hudBar(
		sf::Vector2f(static_cast<float>(windowSize.x), hudHeight));
	hudBar.setPosition(0.0f, static_cast<float>(windowSize.y) - hudHeight);
	hudBar.setFillColor(sf::Color(12, 22, 38, 150));
	
	window.draw(hudBar);
	
	m_speedWidget.setSpeedMetersPerSecond(m_speedMetersPerSecond);
	m_speedWidget.setPosition(110.0f, gaugeCenterY);
	m_speedWidget.setRadius(72.0f);
	m_speedWidget.draw(window);
	
	m_compassWidget.setHeadingDegrees(m_headingDegrees);
	m_compassWidget.setPosition(290.0f, gaugeCenterY);
	m_compassWidget.setRadius(78.0f);
	m_compassWidget.draw(window);
	
	m_verticalSpeedWidget.setVerticalSpeedMetersPerSecond(m_verticalSpeedMetersPerSecond);
	m_verticalSpeedWidget.setPosition(static_cast<float>(windowSize.x) * 0.5f, gaugeCenterY);
	m_verticalSpeedWidget.setRadius(74.0f);
	m_verticalSpeedWidget.draw(window);
	
	m_aglWidget.setAltitudeAboveGroundMeters(m_altitudeAboveGroundMeters);
	m_aglWidget.setPosition(static_cast<float>(windowSize.x) - 300.0f, gaugeCenterY);
	m_aglWidget.setRadius(72.0f);
	m_aglWidget.draw(window);
	
	m_altimeterWidget.setAltitudeMeters(m_altitudeMeters);
	m_altimeterWidget.setPosition(static_cast<float>(windowSize.x) - 120.0f, gaugeCenterY);
	m_altimeterWidget.setRadius(88.0f);
	m_altimeterWidget.draw(window);
}
