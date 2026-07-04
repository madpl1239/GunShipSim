#pragma once

#include <SFML/Graphics.hpp>

class AGLWidget
{
public:
	AGLWidget();
	
	void setFont(const sf::Font *font);
	void setAltitudeAboveGroundMeters(float altitudeAboveGroundMeters);
	void setPosition(float x, float y);
	void setRadius(float radius);
	
	void draw(sf::RenderTarget &target) const;
	
private:
	const sf::Font *m_font;
	float m_altitudeAboveGroundMeters;
	float m_centerX;
	float m_centerY;
	float m_radius;
};
