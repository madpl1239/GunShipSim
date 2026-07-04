/*
 * HelicopterInput.cpp
 * 
 * 04-07-2026 by madpl
 */
#include <SFML/Window/Keyboard.hpp>
#include <helicopter/HelicopterInput.hpp>


HelicopterInputState HelicopterInput::readInput() const
{
	HelicopterInputState state{};
	state.forwardInput = 0.0f;
	state.yawInput = 0.0f;
	state.verticalInput = 0.0f;
	state.brake = false;
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		state.forwardInput += 1.0f;
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		state.forwardInput -= 1.0f;
		state.brake = true;
	}
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		state.yawInput -= 1.0f;
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		state.yawInput += 1.0f;
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
		state.verticalInput += 1.0f;
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::E))
		state.verticalInput -= 1.0f;
	
	return state;
}
