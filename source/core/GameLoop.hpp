/*
 * GameLoop.hpp
 * 
 * 05-07-2026 by madpl
 */
#pragma once

#include <core/Game.hpp>


class GameLoop
{
public:
	explicit GameLoop(Game& game):
		m_game(game)
	{
		// empty
	}
	
	void run();
	
private:
	Game& m_game;
	static constexpr float FIXED_DT = 1.0f / 60.0f;
};
