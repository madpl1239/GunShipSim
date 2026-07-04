/*
 * HelicopterInput.hpp
 * 
 * 04-07-2026 by madpl
 */
#pragma once


struct HelicopterInputState
{
	float forwardInput;
	float yawInput;
	float verticalInput;
	bool brake;
};


class HelicopterInput
{
public:
	HelicopterInputState readInput() const;
};
