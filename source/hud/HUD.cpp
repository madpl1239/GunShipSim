/*
 * HUD.cpp
 * 
 * 04-07-2026 by madpl
 */
#include <sstream>
#include <hud/HUD.hpp>


HUD::HUD():
	m_isInitialized(false),
	m_headingDegrees(0.0f),
	m_altitudeMeters(0.0f),
	m_altitudeAboveGroundMeters(0.0f),
	m_speedMetersPerSecond(0.0f),
	m_verticalSpeedMetersPerSecond(0.0f)
{
	// empty
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
		return;
	
	sf::Vector2u windowSize = window.getSize();
	
	float hudHeight = 202.5f;
	float gaugeCenterY = static_cast<float>(windowSize.y) - hudHeight * 0.5f;
	
	sf::RectangleShape hudBar(sf::Vector2f(static_cast<float>(windowSize.x), hudHeight));
	
	hudBar.setPosition(0.0f, static_cast<float>(windowSize.y) - hudHeight);
	hudBar.setFillColor(sf::Color(12, 22, 58, 60));
	
	window.draw(hudBar);
	
	m_compassWidget.setHeadingDegrees(m_headingDegrees);
	m_compassWidget.setPosition(155.0f, gaugeCenterY);
	m_compassWidget.setRadius(78.0f);
	m_compassWidget.draw(window);
	
	m_altimeterWidget.setAltitudeMeters(m_altitudeMeters);
	m_altimeterWidget.setPosition(static_cast<float>(windowSize.x) - 170.0f, gaugeCenterY);
	m_altimeterWidget.setRadius(88.0f);
	m_altimeterWidget.draw(window);
	
	std::ostringstream ss;
	ss << "SPD " << static_cast<int>(m_speedMetersPerSecond) << " m/s"
		<< "   AGL " << static_cast<int>(m_altitudeAboveGroundMeters) << " m"
		<< "   VS " << static_cast<int>(m_verticalSpeedMetersPerSecond) << " m/s";
	
	sf::Text infoText;
	infoText.setFont(m_font);
	infoText.setCharacterSize(30);
	infoText.setString(ss.str());
	infoText.setFillColor(sf::Color(180, 255, 180));
	
	sf::FloatRect bounds = infoText.getLocalBounds();
	infoText.setOrigin(bounds.left + bounds.width * 0.5f, bounds.top + bounds.height * 0.5f);
	infoText.setPosition(static_cast<float>(windowSize.x) * 0.5f,
						 static_cast<float>(windowSize.y) - 56.0f);
	
	window.draw(infoText);
}
