/*
 * NetworkConfig.hpp
 * 
 * 06-07-2026 by madpl
 */
#pragma once

#include <cstdint>
#include <string>


enum class NetworkMode
{
	Local,
	Host,
	Client
};


struct NetworkConfig
{
	NetworkMode mode{NetworkMode::Local};
	std::string ipAddress{"127.0.0.1"};
	std::uint16_t port{55001};
};
