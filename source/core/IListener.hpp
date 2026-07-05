/*
 * IListener.hpp
 *
 * 05-07-2026 by madpl
 */
#pragma once

#include <core/Event.hpp>


class IListener
{
public:
	virtual ~IListener() = default;
	
	virtual void onEvent(const Event& event) = 0;
};
