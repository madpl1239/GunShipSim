#pragma once

#include <SFML/Graphics.hpp>

class CompassWidget
{
public:
	CompassWidget();
	
	void setFont(const sf::Font *font);
	void setHeadingDegrees(float headingDegrees);
	void setPosition(float x, float y);
	void setRadius(float radius);
	
	void draw(sf::RenderTarget &target) const;
	
private:
	const sf::Font *m_font;
	float m_headingDegrees;
	float m_centerX;
	float m_centerY;
	float m_radius;
};
