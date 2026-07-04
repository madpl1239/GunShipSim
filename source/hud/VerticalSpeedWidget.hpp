/*
 * VerticalSpeedWidget.hpp
 * 
 * 04-07-2026 by madpl
 */
#pragma once

#include <SFML/Graphics.hpp>


class VerticalSpeedWidget
{
public:
	VerticalSpeedWidget();
	
	void setFont(const sf::Font* font);
	void setVerticalSpeedMetersPerSecond(float verticalSpeedMetersPerSecond);
	void setPosition(float x, float y);
	void setRadius(float radius);
	
	void draw(sf::RenderTarget& target) const;
	
private:
	const sf::Font* m_font;
	float m_verticalSpeedMetersPerSecond;
	float m_centerX;
	float m_centerY;
	float m_radius;
};
