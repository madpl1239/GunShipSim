/*
 * NetCodec.hpp
 * 
 * 06-07-2026 by madpl
 */
#pragma once

#include <vector>
#include <cstddef>
#include <cstdint>
#include <network/NetPacket.hpp>


class NetCodec
{
public:
	static std::vector<std::uint8_t> encode(const InputSnapshotPacket& packet);
	static bool decode(const std::uint8_t* data, std::size_t size, InputSnapshotPacket& outPacket);
	
	static std::vector<std::uint8_t> encode(const StateSnapshotPacket& packet);
	static bool decode(const std::uint8_t* data, std::size_t size, StateSnapshotPacket& outPacket);
};
