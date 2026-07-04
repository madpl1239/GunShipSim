/*
 * VerticalSpeedWidget.cpp
 */

#include <hud/VerticalSpeedWidget.hpp>

#include <cmath>
#include <string>

VerticalSpeedWidget::VerticalSpeedWidget()
: m_font(nullptr),
m_verticalSpeedMetersPerSecond(0.0f),
m_centerX(0.0f),
m_centerY(0.0f),
m_radius(40.0f)
{
}

void VerticalSpeedWidget::setFont(const sf::Font *font)
{
	m_font = font;
}

void VerticalSpeedWidget::setVerticalSpeedMetersPerSecond(float verticalSpeedMetersPerSecond)
{
	m_verticalSpeedMetersPerSecond = verticalSpeedMetersPerSecond;
}

void VerticalSpeedWidget::setPosition(float x, float y)
{
	m_centerX = x;
	m_centerY = y;
}

void VerticalSpeedWidget::setRadius(float radius)
{
	m_radius = radius;
}

void VerticalSpeedWidget::draw(sf::RenderTarget &target) const
{
	sf::CircleShape bezel(m_radius);
	bezel.setOrigin(m_radius, m_radius);
	bezel.setPosition(m_centerX, m_centerY);
	bezel.setFillColor(sf::Color(15, 18, 22, 210));
	bezel.setOutlineThickness(3.0f);
	bezel.setOutlineColor(sf::Color(170, 190, 170));
	
	target.draw(bezel);
	
	for(int i = -4; i <= 4; ++i)
	{
		float value = static_cast<float>(i) * 5.0f;
		float normalized = (value + 20.0f) / 40.0f;
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
		
		line[0].color = (i == 0) ? sf::Color(255, 160, 160) : sf::Color(170, 220, 170);
		line[1].color = (i == 0) ? sf::Color(255, 160, 160) : sf::Color(170, 220, 170);
		
		target.draw(line, 2, sf::Lines);
		
		if(m_font != nullptr)
		{
			sf::Text text;
			text.setFont(*m_font);
			text.setCharacterSize(15);
			
			if(i > 0)
			{
				text.setString("+" + std::to_string(static_cast<int>(value)));
			}
			else
			{
				text.setString(std::to_string(static_cast<int>(value)));
			}
			
			text.setFillColor((i == 0) ? sf::Color(255, 180, 180) : sf::Color(220, 255, 220));
			
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
	
	float clampedSpeed = m_verticalSpeedMetersPerSecond;
	if(clampedSpeed < -20.0f)
	{
		clampedSpeed = -20.0f;
	}
	
	if(clampedSpeed > 20.0f)
	{
		clampedSpeed = 20.0f;
	}
	
	float normalized = (clampedSpeed + 20.0f) / 40.0f;
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
		sf::RectangleShape valueBox(sf::Vector2f(78.0f, 24.0f));
		valueBox.setOrigin(39.0f, 12.0f);
		valueBox.setPosition(m_centerX - 22.0f, m_centerY + 8.0f);
		valueBox.setFillColor(sf::Color(8, 12, 18, 230));
		valueBox.setOutlineThickness(1.0f);
		valueBox.setOutlineColor(sf::Color(100, 140, 100));
		
		target.draw(valueBox);
		
		std::string valueString;
		if(m_verticalSpeedMetersPerSecond > 0.0f)
		{
			valueString += "+";
		}
		
		valueString += std::to_string(static_cast<int>(m_verticalSpeedMetersPerSecond));
		valueString += " m/s";
		
		sf::Text valueText;
		valueText.setFont(*m_font);
		valueText.setCharacterSize(15);
		valueText.setString(valueString);
		valueText.setFillColor(sf::Color(220, 255, 220));
		
		sf::FloatRect valueBounds = valueText.getLocalBounds();
		valueText.setOrigin(
			valueBounds.left + valueBounds.width * 0.5f,
			valueBounds.top + valueBounds.height * 0.5f);
		valueText.setPosition(m_centerX - 22.0f, m_centerY + 8.0f);
		
		target.draw(valueText);
		
		sf::Text label;
		label.setFont(*m_font);
		label.setCharacterSize(13);
		label.setString("VS");
		label.setFillColor(sf::Color(180, 220, 180));
		
		sf::FloatRect labelBounds = label.getLocalBounds();
		label.setOrigin(
			labelBounds.left + labelBounds.width * 0.5f,
			labelBounds.top + labelBounds.height * 0.5f);
		label.setPosition(m_centerX - 20.0f, m_centerY - 26.0f);
		
		target.draw(label);
	}
}
