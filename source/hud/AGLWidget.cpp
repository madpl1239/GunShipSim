/*
 * AGLWidget.cpp
 */

#include <hud/AGLWidget.hpp>

#include <cmath>
#include <string>

AGLWidget::AGLWidget()
: m_font(nullptr),
m_altitudeAboveGroundMeters(0.0f),
m_centerX(0.0f),
m_centerY(0.0f),
m_radius(40.0f)
{
}

void AGLWidget::setFont(const sf::Font *font)
{
	m_font = font;
}

void AGLWidget::setAltitudeAboveGroundMeters(float altitudeAboveGroundMeters)
{
	m_altitudeAboveGroundMeters = altitudeAboveGroundMeters;
}

void AGLWidget::setPosition(float x, float y)
{
	m_centerX = x;
	m_centerY = y;
}

void AGLWidget::setRadius(float radius)
{
	m_radius = radius;
}

void AGLWidget::draw(sf::RenderTarget &target) const
{
	sf::CircleShape bezel(m_radius);
	bezel.setOrigin(m_radius, m_radius);
	bezel.setPosition(m_centerX, m_centerY);
	bezel.setFillColor(sf::Color(15, 18, 22, 210));
	bezel.setOutlineThickness(3.0f);
	bezel.setOutlineColor(sf::Color(170, 190, 170));
	
	target.draw(bezel);
	
	for(int i = 0; i <= 10; ++i)
	{
		float value = i * 20.0f;
		float normalized = value / 200.0f;
		float angleDeg = -140.0f + normalized * 280.0f;
		float angleRad = angleDeg * 3.1415926535f / 180.0f;
		
		float outerR = m_radius - 7.0f;
		float innerR = outerR - 16.0f;
		
		sf::Vertex line[2];
		line[0].position = sf::Vector2f(
			m_centerX + std::cos(angleRad) * innerR,
										m_centerY + std::sin(angleRad) * innerR);
		line[1].position = sf::Vector2f(
			m_centerX + std::cos(angleRad) * outerR,
										m_centerY + std::sin(angleRad) * outerR);
		
		line[0].color = sf::Color(170, 220, 170);
		line[1].color = sf::Color(170, 220, 170);
		
		target.draw(line, 2, sf::Lines);
		
		if(m_font != nullptr and i < 10)
		{
			int labelValue = i * 20;
			
			sf::Text text;
			text.setFont(*m_font);
			text.setCharacterSize(16);
			text.setString(std::to_string(labelValue));
			text.setFillColor(sf::Color(220, 255, 220));
			
			sf::FloatRect bounds = text.getLocalBounds();
			text.setOrigin(
				bounds.left + bounds.width * 0.5f,
				bounds.top + bounds.height * 0.5f);
			text.setPosition(
				m_centerX + std::cos(angleRad) * (m_radius - 31.0f),
							 m_centerY + std::sin(angleRad) * (m_radius - 31.0f));
			
			target.draw(text);
		}
	}
	
	float clampedAltitude = m_altitudeAboveGroundMeters;
	if(clampedAltitude < 0.0f)
	{
		clampedAltitude = 0.0f;
	}
	
	if(clampedAltitude > 200.0f)
	{
		clampedAltitude = 200.0f;
	}
	
	float normalized = clampedAltitude / 200.0f;
	float needleAngleDeg = -140.0f + normalized * 280.0f;
	
	sf::RectangleShape needle(sf::Vector2f(m_radius - 18.0f, 4.0f));
	needle.setOrigin(10.0f, 2.0f);
	needle.setPosition(m_centerX, m_centerY);
	needle.setRotation(needleAngleDeg);
	needle.setFillColor(sf::Color(255, 90, 90));
	
	target.draw(needle);
	
	sf::CircleShape hub(6.0f);
	hub.setOrigin(6.0f, 6.0f);
	hub.setPosition(m_centerX, m_centerY);
	hub.setFillColor(sf::Color(255, 180, 180));
	
	target.draw(hub);
	
	if(m_font != nullptr)
	{
		sf::RectangleShape valueBox(sf::Vector2f(72.0f, 24.0f));
		valueBox.setOrigin(36.0f, 12.0f);
		valueBox.setPosition(m_centerX - 24.0f, m_centerY + 8.0f);
		valueBox.setFillColor(sf::Color(8, 12, 18, 230));
		valueBox.setOutlineThickness(1.0f);
		valueBox.setOutlineColor(sf::Color(100, 140, 100));
		
		target.draw(valueBox);
		
		sf::Text valueText;
		valueText.setFont(*m_font);
		valueText.setCharacterSize(15);
		valueText.setString(std::to_string(static_cast<int>(m_altitudeAboveGroundMeters)) + " m");
		valueText.setFillColor(sf::Color(220, 255, 220));
		
		sf::FloatRect valueBounds = valueText.getLocalBounds();
		valueText.setOrigin(
			valueBounds.left + valueBounds.width * 0.5f,
			valueBounds.top + valueBounds.height * 0.5f);
		valueText.setPosition(m_centerX - 24.0f, m_centerY + 8.0f);
		
		target.draw(valueText);
		
		sf::Text label;
		label.setFont(*m_font);
		label.setCharacterSize(13);
		label.setString("AGL");
		label.setFillColor(sf::Color(180, 220, 180));
		
		sf::FloatRect labelBounds = label.getLocalBounds();
		label.setOrigin(
			labelBounds.left + labelBounds.width * 0.5f,
			labelBounds.top + labelBounds.height * 0.5f);
		label.setPosition(m_centerX - 22.0f, m_centerY - 26.0f);
		
		target.draw(label);
	}
}
