/*
 * Compass.cpp
 * 
 * 04-07-2026 by madpl
 */
#include <cmath>
#include <hud/CompassWidget.hpp>


CompassWidget::CompassWidget():
	m_font(nullptr),
	m_headingDegrees(0.0f),
	m_centerX(0.0f),
	m_centerY(0.0f),
	m_radius(40.0f)
{
	// empty
}


void CompassWidget::setFont(const sf::Font* font)
{
	m_font = font;
}


void CompassWidget::setHeadingDegrees(float headingDegrees)
{
	m_headingDegrees = headingDegrees;
}


void CompassWidget::setPosition(float x, float y)
{
	m_centerX = x;
	m_centerY = y;
}


void CompassWidget::setRadius(float radius)
{
	m_radius = radius;
}


void CompassWidget::draw(sf::RenderTarget& target) const
{
	sf::CircleShape bezel(m_radius);
	bezel.setOrigin(m_radius, m_radius);
	bezel.setPosition(m_centerX, m_centerY);
	bezel.setFillColor(sf::Color(15, 18, 22, 180));
	bezel.setOutlineThickness(2.0f);
	bezel.setOutlineColor(sf::Color(170, 190, 170));
	
	target.draw(bezel);
	
	for(int i = 0; i < 36; ++i)
	{
		float angleDeg = i * 10.0f - m_headingDegrees;
		float angleRad = angleDeg * 3.1415926535f / 180.0f;
		
		float outerR = m_radius - 6.0f;
		float innerR = outerR - ((i % 3 == 0) ? 14.0f : 7.0f);
		
		sf::Vertex line[2];
		line[0].position = sf::Vector2f(m_centerX + std::sin(angleRad) * innerR,
										m_centerY - std::cos(angleRad) * innerR);
		line[1].position = sf::Vector2f(m_centerX + std::sin(angleRad) * outerR,
										m_centerY - std::cos(angleRad) * outerR);
		
		line[0].color = sf::Color(170, 220, 170);
		line[1].color = sf::Color(170, 220, 170);
		
		target.draw(line, 2, sf::Lines);
	}
	
	if(m_font != nullptr)
	{
		struct CompassMark
		{
			const char *label;
			float angle;
		};
		
		CompassMark marks[] =
		{
			{"N", 0.0f},
			{"E", 90.0f},
			{"S", 180.0f},
			{"W", 270.0f}
		};
		
		for(const CompassMark &mark : marks)
		{
			float angleDeg = mark.angle - m_headingDegrees;
			float angleRad = angleDeg * 3.1415926535f / 180.0f;
			float textR = m_radius - 24.0f;
			
			sf::Text text;
			text.setFont(*m_font);
			text.setCharacterSize(16);
			text.setString(mark.label);
			text.setFillColor(sf::Color(220, 255, 220));
			
			sf::FloatRect bounds = text.getLocalBounds();
			text.setOrigin(bounds.left + bounds.width * 0.5f,
						   bounds.top + bounds.height * 0.5f);
			text.setPosition(m_centerX + std::sin(angleRad) * textR,
							 m_centerY - std::cos(angleRad) * textR);
			
			target.draw(text);
		}
		
		sf::Text label;
		label.setFont(*m_font);
		label.setCharacterSize(14);
		label.setString("HDG");
		label.setFillColor(sf::Color(180, 220, 180));
		label.setPosition(m_centerX - 18.0f, m_centerY + m_radius - 64.0f);
		
		target.draw(label);
	}
	
	sf::ConvexShape pointer;
	pointer.setPointCount(3);
	pointer.setPoint(0, sf::Vector2f(m_centerX, m_centerY - m_radius + 6.0f));
	pointer.setPoint(1, sf::Vector2f(m_centerX - 8.0f, m_centerY - m_radius + 14.0f));
	pointer.setPoint(2, sf::Vector2f(m_centerX + 8.0f, m_centerY - m_radius + 14.0f));
	pointer.setFillColor(sf::Color(255, 120, 120));
	
	target.draw(pointer);
}
